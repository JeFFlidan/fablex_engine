#include "task_composer.h"
#include "platform/platform.h"
#include "logger.h"
#include <algorithm>

namespace fe
{

uint32 get_priority_count()
{
    return uint32(TaskGroup::Priority::COUNT);
}

void TaskComposer::init(uint32 maxThreadCount)
{
    FE_LOG(LogTasks, INFO, "Starting Task Composer initialization");

    maxThreadCount = std::max(1u, maxThreadCount);
    uint32 coreCount = std::thread::hardware_concurrency();

    for (uint32 i = 0; i != get_priority_count(); ++i)
    {
        TaskGroup::Priority priority = (TaskGroup::Priority)i;
        PriorityContext& priorityCtx = s_priorityContexts.at(i);

        switch (priority)
        {
        case TaskGroup::Priority::HIGH:
            priorityCtx.threadCount = coreCount - 1;
            break;
        case TaskGroup::Priority::LOW:
            priorityCtx.threadCount = coreCount - 2;
            break;
        case TaskGroup::Priority::STREAMING:
            priorityCtx.threadCount = 1;
            break;
        default:
            FE_CHECK(0);
            break;
        }

        priorityCtx.threadCount = std::clamp(priorityCtx.threadCount, 1u, maxThreadCount);
        priorityCtx.taskQueueGroup.reset(new TaskQueueGroup(priorityCtx.threadCount));
        priorityCtx.threads.reserve(priorityCtx.threadCount);

        for (uint32 threadID = 0; threadID != priorityCtx.threadCount; ++threadID)
        {
            priorityCtx.threads.emplace_back([threadID, &priorityCtx]
            {
                while (s_isAlive.load())
                {
                    priorityCtx.execute_tasks(threadID);
    
                    std::unique_lock<std::mutex> lock(priorityCtx.mutex);
                    priorityCtx.wakeCondition.wait(lock);
                }
            });

            auto threadHandle = priorityCtx.threads.back().native_handle();
            uint64 core = threadID + 1;
            if (priority == TaskGroup::Priority::STREAMING)
                core = coreCount - 1 - threadID;

#if defined(_WIN32)
            DWORD_PTR affinityMask = 1ull << core;
            DWORD_PTR affinityResult = SetThreadAffinityMask(threadHandle, affinityMask);
            FE_CHECK(affinityResult);
            
            switch (priority)
            {
            case TaskGroup::Priority::HIGH:
            {
                BOOL priorityResult = SetThreadPriority(threadHandle, THREAD_PRIORITY_NORMAL);
                FE_CHECK(priorityResult != 0);

                std::wstring threadName = L"fe::thread_high_" + std::to_wstring(threadID);
                HRESULT result = SetThreadDescription(threadHandle, threadName.c_str());
                FE_CHECK(SUCCEEDED(result));

                break;
            }
            case TaskGroup::Priority::LOW:
            {
                BOOL priorityResult = SetThreadPriority(threadHandle, THREAD_PRIORITY_LOWEST);
                FE_CHECK(priorityResult != 0);

                std::wstring threadName = L"fe::thread_low_" + std::to_wstring(threadID);
                HRESULT result = SetThreadDescription(threadHandle, threadName.c_str());
                FE_CHECK(SUCCEEDED(result));

                break;
            }
            case TaskGroup::Priority::STREAMING:
            {
                BOOL priorityResult = SetThreadPriority(threadHandle, THREAD_PRIORITY_LOWEST);
                FE_CHECK(priorityResult != 0);

                std::wstring threadName = L"fe::thread_streaming_" + std::to_wstring(threadID);
                HRESULT result = SetThreadDescription(threadHandle, threadName.c_str());
                FE_CHECK(SUCCEEDED(result));

                break;
            }
            default:
                FE_CHECK(0);
                break;
            }
#endif
        }
    }

    FE_LOG(LogTasks, INFO, "Task Composer initialization completed. High priority threads: {}; low priority threads: {}; streaming priority threads: {}", 
        get_thread_count(TaskGroup::Priority::HIGH), get_thread_count(TaskGroup::Priority::LOW), get_thread_count(TaskGroup::Priority::STREAMING));
}

void TaskComposer::cleanup()
{
    s_isAlive.store(false);
    bool executeWakeLoop = true;

    std::thread waker([&]()
    {
        while (executeWakeLoop)
            for (PriorityContext& priorityCtx : s_priorityContexts)
                priorityCtx.wakeCondition.notify_all();
    });

    for (PriorityContext& priorityCtx : s_priorityContexts)
        for (auto& thread : priorityCtx.threads)
            thread.join();

    executeWakeLoop = false;
    waker.join();

    for (PriorityContext& priorityCtx : s_priorityContexts)
    {
        priorityCtx.taskQueueGroup.reset();
        priorityCtx.threads.clear();
        priorityCtx.threadCount = 0;
    }
}

void TaskComposer::execute(TaskGroup& taskGroup, const TaskHandler& taskHandler)
{
    taskGroup.increase_task_count(1);

    Task task;
    task.taskHandler = taskHandler;
    task.taskGroup = &taskGroup;
    task.taskSubgroupBeginning = 0;
    task.taskSubgroupEnd = 1;
    task.taskSubgroupID = 0;

    PriorityContext* priorityCtx = get_priority_context(taskGroup.get_priority());
    
    priorityCtx->taskQueueGroup->get_next_queue().push_back(task);
    priorityCtx->wakeCondition.notify_one();
}

void TaskComposer::dispatch(TaskGroup& taskGroup, uint32 taskCount, uint32 groupSize, const TaskHandler& taskHandler)
{
    if (taskCount == 0 || groupSize == 0)
        return;

    PriorityContext* priorityCtx = get_priority_context(taskGroup.get_priority());

    uint32 groupCount = calculate_group_count(taskCount, groupSize);
    taskGroup.increase_task_count(groupCount);

    Task task;
    task.taskHandler = taskHandler;
    task.taskGroup = &taskGroup;
    
    for (auto groupID = 0; groupID != groupCount; ++groupID)
    {
        task.taskSubgroupBeginning = groupID * groupSize;
        task.taskSubgroupEnd = std::min(task.taskSubgroupBeginning + groupSize, taskCount);
        task.taskSubgroupID = groupID;
        priorityCtx->taskQueueGroup->get_next_queue().push_back(task);
    }

    priorityCtx->wakeCondition.notify_one();
}

bool TaskComposer::is_busy(TaskGroup& taskGroup)
{
    return taskGroup.get_pending_task_count() > 0;
}

void TaskComposer::wait(TaskGroup& taskGroup)
{
    if (is_busy(taskGroup))
    {
        PriorityContext* priorityCtx = get_priority_context(taskGroup.get_priority());

        priorityCtx->wakeCondition.notify_all();
        priorityCtx->execute_tasks(priorityCtx->taskQueueGroup->get_next_queue_index());

        while (is_busy(taskGroup))
        {
            std::this_thread::yield();
        }
    }
}

uint32 TaskComposer::calculate_group_count(uint32 taskCount, uint32 groupSize)
{
    return (taskCount + groupSize - 1) / groupSize;
}    

TaskComposer::PriorityContext* TaskComposer::get_priority_context(TaskGroup::Priority priority)
{
    return &s_priorityContexts.at(uint32(priority));
}

void TaskComposer::PriorityContext::execute_tasks(uint32 beginningQueueIndex)
{
    TaskExecutionInfo executionInfo;
    Task task;
    for (auto i = 0; i != threadCount; ++i)
    {
        TaskQueue& taskQueue = taskQueueGroup->get_queue(beginningQueueIndex);
        while (taskQueue.pop_front(task))
        {
            executionInfo.taskSubgroupID = task.taskSubgroupID;

            for (auto taskIndex = task.taskSubgroupBeginning; taskIndex != task.taskSubgroupEnd; ++taskIndex)
            {
                executionInfo.globalTaskIndex = taskIndex;
                executionInfo.taskIndexRelativeToSubgroup = taskIndex - task.taskSubgroupBeginning;
                executionInfo.isFirstTaskInSubgroup = taskIndex == task.taskSubgroupBeginning;
                executionInfo.isLastTaskInSubgroup = taskIndex == task.taskSubgroupEnd - 1;
                task.taskHandler(executionInfo);
            }
            
            task.taskGroup->decrease_task_count(1);
        }

        ++beginningQueueIndex;
    }
}

}