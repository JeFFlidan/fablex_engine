#pragma once

#include "window_ui.h"
#include "core/event.h"
#include <memory>
#include <functional>

namespace fe::editor
{

class WindowCreationRequest : public IEvent
{
public:
    FE_DECLARE_EVENT(WindowCreatedEvent);

    using CreationHandler = std::function<std::unique_ptr<WindowUI>()>;

    WindowCreationRequest(const CreationHandler& handler)
        : m_handler(handler) { }

    std::unique_ptr<WindowUI> create_window() const
    {
        return m_handler();
    }

private:
    CreationHandler m_handler;
};

}