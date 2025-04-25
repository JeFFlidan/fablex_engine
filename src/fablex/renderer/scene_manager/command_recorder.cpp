#include "command_recorder.h"

namespace fe::renderer
{

void CommandRecorder::set_cmd_buffer(rhi::CommandBuffer* newCmdBuffer)
{
    FE_CHECK(newCmdBuffer);
    m_cmd = newCmdBuffer;
}

void CommandRecorder::record(const CmdRecordHandler& handler) const
{
    std::scoped_lock<std::mutex> locker(m_mutex);
    handler(m_cmd);
}

}