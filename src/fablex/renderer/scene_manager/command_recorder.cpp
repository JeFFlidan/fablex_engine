#include "command_recorder.h"
#include "core/macro.h"

namespace fe::renderer
{

void CommandRecorder::set_cmd(CommandBufferRef newCmdBuffer) const
{
    FE_CHECK(newCmdBuffer);
    m_cmd = newCmdBuffer;
}

void CommandRecorder::record(const CmdRecordHandler& handler) const
{
    FE_CHECK(m_cmd);
    std::scoped_lock<std::mutex> locker(m_mutex);
    handler(m_cmd);
}

void CommandRecorder::record_no_sync(const CmdRecordHandler& handler) const
{
    FE_CHECK(m_cmd);
    handler(m_cmd);
}

}