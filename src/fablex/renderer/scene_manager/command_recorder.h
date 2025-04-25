#pragma once

#include "rhi/resources.h"
#include <mutex>
#include <functional>

namespace fe::renderer
{

class CommandRecorder
{
public:
    using CmdRecordHandler = std::function<void(rhi::CommandBuffer*)>;

    void set_cmd_buffer(rhi::CommandBuffer* newCmdBuffer);
    void record(const CmdRecordHandler& handler) const;

private:
    rhi::CommandBuffer* m_cmd;
    mutable std::mutex m_mutex;
};

}