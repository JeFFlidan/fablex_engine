#include "resource_destroyer.h"
#include "globals.h"

namespace fe::renderer
{

void ResourceDestroyer::process_current_frame()
{
    process_handler_array(get_per_frame_handler_array());
}

void ResourceDestroyer::process_all()
{
    for (DestroyHandlerArray& handlers : m_handlersPerFrame)
        process_handler_array(handlers);
}

void ResourceDestroyer::enqueue_destruction(const DestroyHandler& handler)
{
    std::scoped_lock<std::mutex> locker(m_mutexForPerFrameHandlers);
    get_per_frame_handler_array().push_back(handler);
}

void ResourceDestroyer::process_handler_array(DestroyHandlerArray& handlers)
{
    auto begin = handlers.rbegin();
    auto end = handlers.rend();

    for (auto handlerIt = begin; handlerIt != end; ++handlerIt)
        (*handlerIt)();

    handlers.clear();
}

ResourceDestroyer::DestroyHandlerArray& ResourceDestroyer::get_per_frame_handler_array()
{
    if (m_handlersPerFrame.size() < g_frameIndex + 1)
        m_handlersPerFrame.emplace_back();

    return m_handlersPerFrame[g_frameIndex];
}

}
