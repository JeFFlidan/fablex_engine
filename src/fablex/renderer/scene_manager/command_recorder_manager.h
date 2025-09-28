#pragma once

#include "command_recorder.h"
#include "rhi/resources.h"
#include <array>

namespace fe::renderer
{

class CommandRecorderManager
{
public:
    void set_cmd(rhi::CommandBuffer* cmd);
    const CommandRecorder& cmd_recorder(rhi::QueueType queueType) const;

    void record_graphics_cmd(const CommandRecorder::CmdRecordHandler& handler) const;
    void record_compute_cmd(const CommandRecorder::CmdRecordHandler& handler) const;
    void record_transfer_cmd(const CommandRecorder::CmdRecordHandler& handler) const;

private:
    std::array<CommandRecorder, rhi::g_queueCount> m_commandRecorderPerQueue;
};

}