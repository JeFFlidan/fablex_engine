#include "task_composer.h"

namespace fe
{

std::vector<TaskHandler> g_taskHandlers;

void TaskComposer::init(uint32 maxThreadCount)
{
    maxThreadCount = std::max(1u, maxThreadCount);

    uint32 coreCount = std::thread::hardware_concurrency();
    s_threadCount = std::min(maxThreadCount, std::max(1u, coreCount - 1));
    FE_LOG(LogTasks, INFO, "Thread count: {}", s_threadCount);
    s_threads.reserve(s_threadCount);
    s_taskQueueGroup = std::make_unique<TaskQueueGroup>(s_threadCount);

    for (auto threadID = 0; threadID != s_threadCount; ++threadID)
    {
        s_threads.emplace_back([threadID]
        {
            while (s_isAlive.load())
            {
                execute_tasks(threadID);

                std::unique_lock<std::mutex> lock(s_mutex);
                s_wakeCondition.wait(lock);
            }
        });
    }
}

void TaskComposer::cleanup()
{
    s_isAlive.store(false);
    bool executeWakeLoop = true;

    std::thread waker([&]()
    {
        while (executeWakeLoop)
            s_wakeCondition.notify_all();
    });

    for (auto& thread : s_threads)
    {
        thread.join();
    }

    executeWakeLoop = false;
    waker.join();
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

    s_taskQueueGroup->get_next_queue().push_back(task);
    s_wakeCondition.notify_one();
}

void TaskComposer::dispatch(TaskGroup& taskGroup, uint32 taskCount, uint32 groupSize, const TaskHandler& taskHandler)
{
    if (taskCount == 0 || groupSize == 0)
        return;

    uint32 groupCount = calculate_group_count(taskCount, groupSize);
    taskGroup.increase_task_count(groupCount);

    Task task;
    task.taskHandler = taskHandler;
    task.taskGroup = &taskGroup;
    g_taskHandlers.push_back(taskHandler);
    
    for (auto groupID = 0; groupID != groupCount; ++groupID)
    {
        task.taskSubgroupBeginning = groupID * groupSize;
        task.taskSubgroupEnd = std::min(task.taskSubgroupBeginning + groupSize, taskCount);
        task.taskSubgroupID = groupID;
        s_taskQueueGroup->get_next_queue().push_back(task);
    }

    s_wakeCondition.notify_all();
}

bool TaskComposer::is_busy(TaskGroup& taskGroup)
{
    return taskGroup.get_pending_task_count() > 0;
}

void TaskComposer::wait(TaskGroup& taskGroup)
{
    if (is_busy(taskGroup))
    {
        s_wakeCondition.notify_all();
        execute_tasks(s_taskQueueGroup->get_next_queue_index());

        while (is_busy(taskGroup))
        {
            std::this_thread::yield();
        }
    }
}

void TaskComposer::execute_tasks(uint32 beginningQueueIndex)
{
    TaskExecutionInfo executionInfo;
    Task task;
    for (auto i = 0; i != s_threadCount; ++i)
    {
        TaskQueue& taskQueue = s_taskQueueGroup->get_queue(beginningQueueIndex);
        while (taskQueue.pop_front(task))
        {
            task.taskGroup->wait_for_other_groups();
            executionInfo.taskSubgroupID = task.taskSubgroupID;

            for (auto taskIndex = task.taskSubgroupBeginning; taskIndex != task.taskSubgroupEnd; ++taskIndex)
            {
                executionInfo.globalTaskIndex = taskIndex;
                executionInfo.taskIndexRelativeToSubgroup = taskIndex - task.taskSubgroupBeginning;
                executionInfo.isFirstTaskInSubgroup = taskIndex == task.taskSubgroupBeginning;
                executionInfo.isLastTaskInSubgroup = taskIndex == task.taskSubgroupEnd - 1;
                try
                {
                    task.taskHandler(executionInfo);
                }
                catch (std::bad_function_call)
                {
                    FE_LOG(LogTasks, FATAL, "TaskComposer::execute_tasks(): Bad function call. Task index: {}", executionInfo.globalTaskIndex);
                }
            }
            
            task.taskGroup->decrease_task_count(1);
        }

        ++beginningQueueIndex;
    }
}

uint32 TaskComposer::calculate_group_count(uint32 taskCount, uint32 groupSize)
{
    return (taskCount + groupSize - 1) / groupSize;
}    

}