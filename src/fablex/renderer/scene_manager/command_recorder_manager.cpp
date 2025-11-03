#include "command_recorder_manager.h"
#include "rhi/resources/cmd.h"
#include "rhi/utils.h"

namespace fe::renderer
{

void CommandRecorderManager::set_cmd(CommandBufferRef cmd)
{
    cmd_recorder(cmd->cmdPool->queueType).set_cmd(cmd);
}

const CommandRecorder& CommandRecorderManager::cmd_recorder(QueueType queueType) const
{
    return m_commandRecorderPerQueue[get_queue_index(queueType)];
}

void CommandRecorderManager::record_graphics_cmd(const CommandRecorder::CmdRecordHandler& handler) const
{
    cmd_recorder(QueueType::GRAPHICS).record(handler);
}

void CommandRecorderManager::record_compute_cmd(const CommandRecorder::CmdRecordHandler& handler) const
{
    cmd_recorder(QueueType::COMPUTE).record(handler);
}

void CommandRecorderManager::record_transfer_cmd(const CommandRecorder::CmdRecordHandler& handler) const
{
    cmd_recorder(QueueType::TRANSFER).record(handler);
}

}