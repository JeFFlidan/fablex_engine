#pragma once

#include "handles/command_pool.h"
#include "handles/command_buffer.h"
#include "handles/handle_vector.h"

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
    using CommandBufferArray = std::vector<CommandBufferRef>;

    class HandleStorage
    {
    public:
        CommandPoolRef create_cmd_pool(const CommandPoolCreateInfo& info);
        CommandBufferRef create_cmd_buffer(const CommandBufferCreateInfo& info);

    private:
        HandleVector<CommandPoolHandle> m_cmdPools;
        HandleVector<CommandBufferHandle> m_cmdBuffers;
    };

    struct CommandPoolContext
    {
        CommandPoolRef cmdPool;
        CommandBufferArray freeCmdBuffers;
        CommandBufferArray usedCmdBuffers;

        CommandPoolContext() = default;
    };

    using CommandPoolContextArray = std::vector<CommandPoolContext>;

    class CommandAllocator
    {
    public:
        CommandAllocator(HandleStorage& storage);
        ~CommandAllocator();

        void reset();
        void cleanup();

        CommandBufferRef get_cmd(QueueType queueType, HandleStorage& storage);

    private:
        CommandPoolContextArray m_cmdPoolContextPerQueue;
    };

    using CommandAllocatorArray = std::vector<CommandAllocator>;
    using ThreadID = std::thread::id;

    HandleStorage m_handleStorage;

    uint32 m_freeAllocatorIndex = 0;
    std::mutex m_mutex;
    std::vector<CommandAllocatorArray> m_freeAllocatorsPerFrame;
    std::unordered_map<ThreadID, uint32> m_allocatorIndexPerThread;
};

}