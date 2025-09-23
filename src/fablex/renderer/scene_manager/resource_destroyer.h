#pragma once

#include <mutex>
#include <vector>
#include <functional>

namespace fe::renderer
{

using DestroyHandler = std::function<void()>;

class ResourceDestroyer
{
public:
    void process_current_frame();
    void process_all();

    void enqueue_destruction(const DestroyHandler& handler);

private:
    using DestroyHandlerArray = std::vector<DestroyHandler>;

    std::vector<DestroyHandlerArray> m_handlersPerFrame;
    std::mutex m_mutexForPerFrameHandlers;

    void process_handler_array(DestroyHandlerArray& handlers);

    DestroyHandlerArray& get_per_frame_handler_array();
};

}