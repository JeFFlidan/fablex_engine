#pragma once

#include "types.h"
#include "logger.h"

#include <functional>
#include <unordered_set>
#include <mutex>
#include <deque>

namespace fe
{

DEFINE_LOG_CATEGORY(LogTasks);

struct TaskExecutionInfo
{
    uint32 globalTaskIndex;
    uint32 taskSubgroupID;
    uint32 taskIndexRelativeToSubgroup;
    bool isFirstTaskInSubgroup;
    bool isLastTaskInSubgroup;
};

using TaskHandler = std::function<void(TaskExecutionInfo)>;

class TaskGroup
{
    public:
        void increase_task_count(uint32_t taskCount)
        {
            m_pendingTaskCount.fetch_add(taskCount);
            m_isDependenciesResolved = false;
        }

        void decrease_task_count(uint32_t taskCount)
        {
            m_pendingTaskCount.fetch_sub(taskCount);
            if (m_pendingTaskCount.load() == 0)
            {
                m_waitCondition.notify_all();
                m_isDependenciesResolved = true;
            }
        }

        uint32_t get_pending_task_count()
        {
            return m_pendingTaskCount.load();
        }

        void wait_for_other_groups()
        {
            if (!m_isDependenciesResolved)
            {
                for (auto& dependency : m_dependencies)
                {
                    std::unique_lock<std::mutex> lock(dependency->m_mutex);
                    dependency->m_waitCondition.wait(lock);
                }
            }
        }

        void add_dependency(TaskGroup* taskGroup)
        {
            if (taskGroup)
                m_dependencies.insert(taskGroup);
        }

        void remove_dependency(TaskGroup* taskGroup)
        {
            auto it = m_dependencies.find(taskGroup);
            if (it == m_dependencies.end())
                FE_LOG(LogTasks, FATAL, "TaskGroup::remove_dependency(): Task group doesn't have dependency that you want to remove");
            m_dependencies.erase(taskGroup);
        }
    
    private:
        std::atomic<uint32_t> m_pendingTaskCount{ 0 };
        std::condition_variable m_waitCondition;
        std::mutex m_mutex;
        std::unordered_set<TaskGroup*> m_dependencies;		// Task group waits for these task groups
        bool m_isDependenciesResolved{ true };
};

struct Task
{
    TaskHandler taskHandler;
    TaskGroup* taskGroup;
    uint32_t taskSubgroupID;
    uint32_t taskSubgroupBeginning;
    uint32_t taskSubgroupEnd;
};

class TaskQueue
{
    public:
        void push_back(const Task& task)
        {
            std::scoped_lock<std::mutex> lock(m_mutex);
            m_tasks.push_back(task);
        }

        bool pop_front(Task& task)
        {
            std::scoped_lock<std::mutex> lock(m_mutex);
            if (m_tasks.empty())
                return false;
            task = std::move(m_tasks.front());
            m_tasks.pop_front();
            return true;
        }

        bool empty()
        {
            std::scoped_lock<std::mutex> lock(m_mutex);
            return m_tasks.empty();
        }
    
    private:
        std::deque<Task> m_tasks;
        std::mutex m_mutex;
};

class TaskQueueGroup
{
    public:
        TaskQueueGroup(uint32_t threadCount) : m_threadCount(threadCount)
        {
            m_taskQueues.reserve(threadCount);
            for (auto i = 0; i != threadCount; ++i)
                m_taskQueues.emplace_back(new TaskQueue());
        }

        TaskQueue& get_queue(uint32_t index)
        {
            return *m_taskQueues[index % m_threadCount];
        }
    
        TaskQueue& get_next_queue()
        {
            return *m_taskQueues[m_nextQueue.fetch_add(1) % m_threadCount];
        }

        uint32_t get_next_queue_index()
        {
            return m_nextQueue.fetch_add(1) % m_threadCount;
        }

    private:
        std::vector<std::unique_ptr<TaskQueue>> m_taskQueues;
        std::atomic<uint64> m_nextQueue = 0;
        uint32_t m_threadCount;
};

}