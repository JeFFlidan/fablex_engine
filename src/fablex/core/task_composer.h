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
        explicit TaskComposer(uint32 threadCount = ~0u);
        ~TaskComposer();
        void execute(TaskGroup& taskGroup, const TaskHandler& taskHandler);
        void dispatch(TaskGroup& taskGroup, uint32 taskCount, uint32 groupSize, const TaskHandler& taskHandler);
        bool is_busy(TaskGroup& taskGroup);
        void wait(TaskGroup& taskGroup);

        TaskGroup* allocate_task_group()
        {
            return m_taskGroupPool.allocate();
        }

        void free_task_group(TaskGroup* taskGroup)
        {
            return m_taskGroupPool.free(taskGroup);
        }

        uint32 get_thread_count()
        {
            return m_threadCount;
        }
    
    private:
        std::unique_ptr<TaskQueueGroup> m_taskQueueGroup{ nullptr };
        ThreadSafePoolAllocator<TaskGroup, 128> m_taskGroupPool;
        std::vector<std::thread> m_threads;
        std::condition_variable m_wakeCondition;
        std::mutex m_mutex;
        uint32 m_threadCount;
        std::atomic_bool m_isAlive{ true };

        void execute_tasks(uint32 beginningQueueIndex);
        uint32 calculate_group_count(uint32 taskCount, uint32 groupSize);
}; 
}