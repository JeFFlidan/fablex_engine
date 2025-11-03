#pragma once

#include "handles/command_pool.h"
#include "handles/command_buffer.h"

#include <thread>
#include <unordered_map>
#include <mutex>

namespace fe::renderer
{

class CommandManager
{
public:
    ~CommandManager();

    void begin_frame();
    void end_frame();

    CommandBufferRef get_cmd(QueueType queueType);

private:
    using CommandBufferArray = std::vector<CommandBufferHandle>;

    struct CommandPoolContext
    {
        CommandPoolHandle cmdPool;
        CommandBufferArray freeCmdBuffers;
        CommandBufferArray usedCmdBuffers;

        CommandPoolContext() = default;

        CommandPoolContext(CommandPoolContext&& other) noexcept
            : cmdPool(std::move(other.cmdPool)),
            freeCmdBuffers(std::move(other.freeCmdBuffers)),
            usedCmdBuffers(std::move(other.usedCmdBuffers)) { }

        CommandPoolContext& operator=(CommandPoolContext&& other) noexcept
        {
            if (this != &other)
            {
                cmdPool = std::move(other.cmdPool);
                freeCmdBuffers = std::move(other.freeCmdBuffers);
                usedCmdBuffers = std::move(other.usedCmdBuffers);
            }
            return *this;
        }

        CommandPoolContext(const CommandPoolContext&) = delete;
        CommandPoolContext& operator=(const CommandPoolContext&) = delete;
    };

    using CommandPoolContextArray = std::vector<CommandPoolContext>;

    class CommandAllocator
    {
    public:
        CommandAllocator();
        ~CommandAllocator();

        CommandAllocator(CommandAllocator&& other) noexcept
            : m_cmdPoolContextPerQueue(std::move(other.m_cmdPoolContextPerQueue)) { }

        CommandAllocator& operator=(CommandAllocator&& other) noexcept
        {
            if (this != &other)
            {
                m_cmdPoolContextPerQueue = std::move(other.m_cmdPoolContextPerQueue);
            }
            return *this;
        }

        void reset();
        void cleanup();

        CommandBufferRef get_cmd(QueueType queueType);

    private:
        CommandPoolContextArray m_cmdPoolContextPerQueue;
    };

    using CommandAllocatorArray = std::vector<CommandAllocator>;
    using ThreadID = std::thread::id;

    uint32 m_freeAllocatorIndex = 0;
    std::mutex m_mutex;
    std::vector<CommandAllocatorArray> m_freeAllocatorsPerFrame;
    std::unordered_map<ThreadID, uint32> m_allocatorIndexPerThread;
};

}