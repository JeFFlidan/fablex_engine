#pragma once

#include "rhi/resources.h"
#include <thread>
#include <unordered_map>

namespace fe::renderer
{

class CommandManager
{
public:
    void begin_frame();
    void cleanup();

    rhi::CommandBuffer* get_command_buffer(rhi::QueueType queueType);

private:
    using CommandBufferArray = std::vector<rhi::CommandBuffer*>;

    struct CommandPoolContext
    {
        rhi::CommandPool* cmdPool;
        CommandBufferArray freeCmdBuffers;
        CommandBufferArray usedCmdBuffers;
    };

    using CommandPoolContextArray = std::vector<CommandPoolContext>;

    class CommandAllocator
    {
    public:
        CommandAllocator();
        ~CommandAllocator();

        void reset();
        void cleanup();

        rhi::CommandBuffer* get_command_buffer(rhi::QueueType queueType);

    private:
        CommandPoolContextArray m_cmdPoolContextPerQueue;
    };

    using CommandAllocatorArray = std::vector<CommandAllocator>;
    using ThreadID = std::thread::id;

    uint32 m_freeAllocatorIndex = 0;
    std::vector<CommandAllocatorArray> m_freeAllocatorsPerFrame;
    std::unordered_map<ThreadID, uint32> m_allocatorIndexPerThread;
};

}