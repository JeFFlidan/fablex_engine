#include "command_manager.h"
#include "globals.h"
#include "rhi/rhi.h"
#include "rhi/utils.h"
#include <sstream>

namespace fe::renderer
{

std::string to_string(std::thread::id id) {
    std::ostringstream oss;
    oss << id;
    return oss.str();
}

CommandManager::~CommandManager()
{
    m_freeAllocatorsPerFrame.clear();
}

void CommandManager::begin_frame()
{
    m_allocatorIndexPerThread.clear();
    m_freeAllocatorIndex = 0;

    if (m_freeAllocatorsPerFrame.size() < g_frameIndex + 1)
        m_freeAllocatorsPerFrame.emplace_back();

    CommandAllocatorArray& cmdAllocators = m_freeAllocatorsPerFrame.at(g_frameIndex);
    cmdAllocators.reserve(std::thread::hardware_concurrency());

    for (CommandAllocator& cmdAllocator : cmdAllocators)
        cmdAllocator.reset();
}

void CommandManager::end_frame()
{
    
}

rhi::CommandBuffer* CommandManager::get_cmd(rhi::QueueType queueType)
{
    ThreadID curThreadID = std::this_thread::get_id();
    CommandAllocatorArray& cmdAllocators = m_freeAllocatorsPerFrame.at(g_frameIndex);
    
    std::scoped_lock<std::mutex> locker(m_mutex);

    auto allocIdxIt = m_allocatorIndexPerThread.find(curThreadID);
    uint64 allocIdx = 0;

    if (allocIdxIt == m_allocatorIndexPerThread.end())
    {
        if (m_freeAllocatorIndex + 1 > cmdAllocators.size())
            cmdAllocators.emplace_back();

        m_allocatorIndexPerThread[curThreadID] = m_freeAllocatorIndex;
        allocIdx = m_freeAllocatorIndex++;
    }
    else 
    {
        allocIdx = allocIdxIt->second;
    }

    FE_CHECK(cmdAllocators.size() <= std::thread::hardware_concurrency());
    
    return cmdAllocators.at(allocIdx).get_cmd(queueType);
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

        std::string name = "CmdPool_" + to_string(std::this_thread::get_id()) + "_" + std::to_string(g_frameIndex);
        rhi::set_name(cmdPoolContext.cmdPool, name.c_str());
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

rhi::CommandBuffer* CommandManager::CommandAllocator::get_cmd(rhi::QueueType queueType)
{
    CommandPoolContext& cmdPoolContext = m_cmdPoolContextPerQueue.at(rhi::get_queue_index(queueType));

    if (cmdPoolContext.freeCmdBuffers.empty())
    {
        rhi::CommandBufferInfo info;
        info.cmdPool = cmdPoolContext.cmdPool;
        rhi::create_command_buffer(&cmdPoolContext.usedCmdBuffers.emplace_back(), &info);

        std::string name = "CmdBuffer_" + to_string(std::this_thread::get_id()) + "_" + std::to_string(g_frameIndex) + "_" + std::to_string(cmdPoolContext.usedCmdBuffers.size() - 1);
        rhi::set_name(cmdPoolContext.usedCmdBuffers.back(), name.c_str());
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