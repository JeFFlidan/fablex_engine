#pragma once

#include "command_recorder.h"
#include "renderer/globals.h"
#include <array>

namespace fe::renderer
{

class CommandRecorderManager
{
public:
    void set_cmd(CommandBufferRef cmd);
    const CommandRecorder& cmd_recorder(QueueType queueType) const;

    void record_graphics_cmd(const CommandRecorder::CmdRecordHandler& handler) const;
    void record_compute_cmd(const CommandRecorder::CmdRecordHandler& handler) const;
    void record_transfer_cmd(const CommandRecorder::CmdRecordHandler& handler) const;

private:
    std::array<CommandRecorder, g_queueCount> m_commandRecorderPerQueue;
};

}