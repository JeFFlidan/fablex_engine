#include "command_recorder_manager.h"
#include "rhi/utils.h"

namespace fe::renderer
{

void CommandRecorderManager::set_cmd(rhi::CommandBuffer* cmd)
{
    cmd_recorder(cmd->cmdPool->queueType).set_cmd(cmd);
}

const CommandRecorder& CommandRecorderManager::cmd_recorder(rhi::QueueType queueType) const
{
    return m_commandRecorderPerQueue[rhi::get_queue_index(queueType)];
}

void CommandRecorderManager::record_graphics_cmd(const CommandRecorder::CmdRecordHandler& handler) const
{
    cmd_recorder(rhi::QueueType::GRAPHICS).record(handler);
}

void CommandRecorderManager::record_compute_cmd(const CommandRecorder::CmdRecordHandler& handler) const
{
    cmd_recorder(rhi::QueueType::COMPUTE).record(handler);
}

void CommandRecorderManager::record_transfer_cmd(const CommandRecorder::CmdRecordHandler& handler) const
{
    cmd_recorder(rhi::QueueType::TRANSFER).record(handler);
}

}