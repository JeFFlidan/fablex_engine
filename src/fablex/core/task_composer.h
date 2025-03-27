#pragma once

#include "task_types.h"
#include "pool_allocator.h"

#include <condition_variable>
#include <thread>

namespace fe
{

class TaskComposer
{
public:
    static void init(uint32 maxThreadCount = ~0u);
    static void cleanup();
    static void execute(TaskGroup& taskGroup, const TaskHandler& taskHandler);
    static void dispatch(TaskGroup& taskGroup, uint32 taskCount, uint32 groupSize, const TaskHandler& taskHandler);
    static bool is_busy(TaskGroup& taskGroup);
    static void wait(TaskGroup& taskGroup);

    static TaskGroup* allocate_task_group()
    {
        return s_taskGroupPool.allocate();
    }

    static void free_task_group(TaskGroup* taskGroup)
    {
        return s_taskGroupPool.free(taskGroup);
    }

    static uint32 get_thread_count(TaskGroup::Priority priority)
    {
        return get_priority_context(priority)->threadCount;
    }

private:
    struct PriorityContext
    {
        uint32 threadCount;
        std::mutex mutex;
        std::vector<std::thread> threads;
        std::condition_variable wakeCondition;
        std::unique_ptr<TaskQueueGroup> taskQueueGroup;
        
        void execute_tasks(uint32 beginningQueueIndex);
    };

    using PriotityContextArray = std::array<PriorityContext, uint32(TaskGroup::Priority::COUNT)>;

    inline static ThreadSafePoolAllocator<TaskGroup, 128> s_taskGroupPool{};
    inline static PriotityContextArray s_priorityContexts{};
    inline static std::atomic_bool s_isAlive = true;

    static uint32 calculate_group_count(uint32 taskCount, uint32 groupSize);

    static PriorityContext* get_priority_context(TaskGroup::Priority priority);
};

}