#include "deletion_queue.h"
#include "globals.h"

namespace fe::renderer
{

DeletionQueue::~DeletionQueue()
{
    for (const DeletionHandlers& handlers : m_deletionHandlersPerFrame)
        for (auto it = handlers.rbegin(); it != handlers.rend(); ++it)
            (*it)();

    m_deletionHandlersPerFrame.clear();
}

void DeletionQueue::add(const DeletionHandler& deletionHandler)
{
    if (m_deletionHandlersPerFrame.size() < g_frameIndex + 1)
        m_deletionHandlersPerFrame.emplace_back();

    m_deletionHandlersPerFrame.at(g_frameIndex).push_back(deletionHandler);
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