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

    void set_cmd(rhi::CommandBuffer* newCmd) const;
    void record(const CmdRecordHandler& handler) const;
    void record_no_sync(const CmdRecordHandler& handler) const;

private:
    mutable rhi::CommandBuffer* m_cmd;
    mutable std::mutex m_mutex;
};

}