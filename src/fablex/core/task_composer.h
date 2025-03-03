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

        static uint32 get_thread_count()
        {
            return s_threadCount;
        }
    
    private:
        inline static std::unique_ptr<TaskQueueGroup> s_taskQueueGroup = nullptr;
        inline static ThreadSafePoolAllocator<TaskGroup, 128> s_taskGroupPool;
        inline static std::vector<std::thread> s_threads;
        inline static std::condition_variable s_wakeCondition;
        inline static std::mutex s_mutex;
        inline static uint32 s_threadCount;
        inline static std::atomic_bool s_isAlive = true;

        static void execute_tasks(uint32 beginningQueueIndex);
        static uint32 calculate_group_count(uint32 taskCount, uint32 groupSize);
};

}