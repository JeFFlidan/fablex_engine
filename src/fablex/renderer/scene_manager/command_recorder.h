#pragma once

#include "handles/command_buffer.h"

#include <mutex>
#include <functional>

namespace fe::renderer
{

class CommandRecorder
{
public:
    using CmdRecordHandler = std::function<void(CommandBufferRef)>;

    void set_cmd(CommandBufferRef newCmd) const;
    void record(const CmdRecordHandler& handler) const;
    void record_no_sync(const CmdRecordHandler& handler) const;

private:
    mutable CommandBufferRef m_cmd;
    mutable std::mutex m_mutex;
};

}