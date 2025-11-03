#include "command_manager.h"
#include "globals.h"
#include "utils.h"
#include "rhi/rhi.h"
#include "rhi/resources/cmd.h"
#include "rhi/utils.h"
#include <sstream>

namespace fe::renderer
{

std::string to_string(std::thread::id id) {
    std::ostringstream oss;
    oss << id;
    return oss.str();
}

// Name = CmdPool-{ThredID}-{FrameIdx}-{QueueIdx}
void set_cmd_pool_name(CommandPoolRef cmdPool, uint32 queueIdx)
{
    Utils::set_debug_name(cmdPool, "CmdPool-{}-{}-{}", to_string(std::this_thread::get_id()), g_frameIndex, queueIdx);
}

// Name = CmdBuffer-{ThredID}-{FrameIdx}-{QueueIdx}-{CmdIdx}
void set_cmd_name(CommandBufferRef cmd, uint32 queueIdx, uint32 cmdIdx)
{
    Utils::set_debug_name(cmd, "CmdBuffer-{}-{}-{}-{}", to_string(std::this_thread::get_id()), g_frameIndex, queueIdx, cmdIdx);
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

CommandBufferRef CommandManager::get_cmd(QueueType queueType)
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
    m_cmdPoolContextPerQueue.reserve(g_queueCount);
    for (uint32 i = 0; i != g_queueCount; ++i)
    {
        CommandPoolContext& cmdPoolContext = m_cmdPoolContextPerQueue.emplace_back();
        CommandPoolCreateInfo info;
        info.queueType = (QueueType)i;
        cmdPoolContext.cmdPool.init(info);

        set_cmd_pool_name(cmdPoolContext.cmdPool, i);
    }
}

CommandManager::CommandAllocator::~CommandAllocator()
{

}

void CommandManager::CommandAllocator::reset()
{
    for (CommandPoolContext& cmdPoolContext : m_cmdPoolContextPerQueue)
    {
        cmdPoolContext.cmdPool.reset_pool();

        for (CommandBufferHandle& handle : cmdPoolContext.usedCmdBuffers)
            cmdPoolContext.freeCmdBuffers.push_back(std::move(handle));

        cmdPoolContext.usedCmdBuffers.clear();
    }
}

CommandBufferRef CommandManager::CommandAllocator::get_cmd(QueueType queueType)
{
    CommandPoolContext& cmdPoolContext = m_cmdPoolContextPerQueue.at(rhi::get_queue_index(queueType));

    if (cmdPoolContext.freeCmdBuffers.empty())
    {
        CommandBufferCreateInfo info;
        info.cmdPool = cmdPoolContext.cmdPool;
        cmdPoolContext.usedCmdBuffers.emplace_back(info);

        CommandBufferRef cmd = cmdPoolContext.usedCmdBuffers.back();
        uint32 cmdIdx = cmdPoolContext.usedCmdBuffers.size() - 1;
        uint32 queueIdx = std::to_underlying(cmdPoolContext.cmdPool->queueType);

        set_cmd_name(cmd, cmdIdx, queueIdx);
    }
    else
    {
        CommandBufferHandle& cmdBuffer = cmdPoolContext.freeCmdBuffers.back();
        cmdPoolContext.usedCmdBuffers.push_back(std::move(cmdBuffer));
        cmdPoolContext.freeCmdBuffers.pop_back();
    }

    return cmdPoolContext.usedCmdBuffers.back();
}

}