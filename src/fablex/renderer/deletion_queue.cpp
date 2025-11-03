#include "deletion_queue.h"
#include "globals.h"

namespace fe::renderer
{

DeletionQueue::~DeletionQueue()
{
    for (DeletionHandlers& handlers : m_deletionHandlersPerFrame)
        for (auto it = handlers.rbegin(); it != handlers.rend(); ++it)
            (*it)();

    m_deletionHandlersPerFrame.clear();
}

void DeletionQueue::add(DeletionHandler&& deletionHandler)
{
    std::scoped_lock<std::mutex> locker(m_mutex);

    if (m_deletionHandlersPerFrame.size() < g_frameIndex + 1)
    {
        uint32 arrayToCreateCount = g_frameIndex + 1 - m_deletionHandlersPerFrame.size();
        for (uint32 i = 0; i != arrayToCreateCount; ++i)
            m_deletionHandlersPerFrame.emplace_back();
    }

    m_deletionHandlersPerFrame.at(g_frameIndex).emplace_back(std::move(deletionHandler));
}

void DeletionQueue::destroy_objects()
{
    if (m_deletionHandlersPerFrame.size() < g_frameIndex + 1)
        return;

    DeletionHandlers& handlers = m_deletionHandlersPerFrame.at(g_frameIndex);
    for (auto it = handlers.rbegin(); it != handlers.rend(); ++it)
        (*it)();

    handlers.clear();
}

}