#pragma once

#include "rhi/resources.h"
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

    rhi::CommandBuffer* get_cmd(rhi::QueueType queueType);

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

        rhi::CommandBuffer* get_cmd(rhi::QueueType queueType);

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