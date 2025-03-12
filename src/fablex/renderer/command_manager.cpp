#include "command_manager.h"
#include "rhi/rhi.h"
#include "rhi/utils.h"

namespace fe::renderer
{

void CommandManager::begin_frame()
{
    m_allocatorIndexPerThread.clear();
    m_freeAllocatorIndex = 0;

    if (m_freeAllocatorsPerFrame.size() < rhi::get_frame_index() + 1)
        m_freeAllocatorsPerFrame.emplace_back();

    CommandAllocatorArray& cmdAllocators = m_freeAllocatorsPerFrame.at(rhi::get_frame_index());
    for (CommandAllocator& cmdAllocator : cmdAllocators)
        cmdAllocator.reset();
}

void CommandManager::cleanup()
{
    m_freeAllocatorsPerFrame.clear();
}

rhi::CommandBuffer* CommandManager::get_command_buffer(rhi::QueueType queueType)
{
    ThreadID curThreadID = std::this_thread::get_id();
    CommandAllocatorArray& cmdAllocators = m_freeAllocatorsPerFrame.at(rhi::get_frame_index());
    
    auto allocIdxIt = m_allocatorIndexPerThread.find(curThreadID);
    if (allocIdxIt == m_allocatorIndexPerThread.end())
    {
        if (m_freeAllocatorIndex + 1 > cmdAllocators.size())
            cmdAllocators.emplace_back();

        m_allocatorIndexPerThread[curThreadID] = m_freeAllocatorIndex;
    }

    FE_CHECK(cmdAllocators.size() <= std::thread::hardware_concurrency());

    return cmdAllocators.at(m_freeAllocatorIndex++).get_command_buffer(queueType);
}

CommandManager::CommandAllocator::CommandAllocator()
{
    m_cmdPoolContextPerQueue.resize(rhi::g_queueCount);
    for (uint32 i = 0; i != rhi::g_queueCount; ++i)
    {
        CommandPoolContext& cmdPoolContext = m_cmdPoolContextPerQueue.at(i);
        rhi::CommandPoolInfo info;
        info.queueType = (rhi::QueueType)i;
        rhi::create_command_pool(&cmdPoolContext.cmdPool, &info);
    }
}

CommandManager::CommandAllocator::~CommandAllocator()
{
    for (CommandPoolContext& context : m_cmdPoolContextPerQueue)
        rhi::destroy_command_pool(context.cmdPool);
}

void CommandManager::CommandAllocator::reset()
{
    for (CommandPoolContext& cmdPoolContext : m_cmdPoolContextPerQueue)
    {
        rhi::reset_command_pool(cmdPoolContext.cmdPool);

        for (rhi::CommandBuffer* usedCmdBuffer : cmdPoolContext.usedCmdBuffers)
            cmdPoolContext.freeCmdBuffers.push_back(usedCmdBuffer);

        cmdPoolContext.usedCmdBuffers.clear();
    }
}

rhi::CommandBuffer* CommandManager::CommandAllocator::get_command_buffer(rhi::QueueType queueType)
{
    CommandPoolContext& cmdPoolContext = m_cmdPoolContextPerQueue.at(rhi::get_queue_index(queueType));
    
    if (cmdPoolContext.freeCmdBuffers.empty())
    {
        rhi::CommandBufferInfo info;
        info.cmdPool = cmdPoolContext.cmdPool;
        rhi::create_command_buffer(&cmdPoolContext.usedCmdBuffers.emplace_back(), &info);
    }
    else
    {
        rhi::CommandBuffer* cmdBuffer = cmdPoolContext.freeCmdBuffers.back();
        cmdPoolContext.usedCmdBuffers.push_back(cmdBuffer);
        cmdPoolContext.freeCmdBuffers.pop_back();
    }

    return cmdPoolContext.usedCmdBuffers.back();
}

}