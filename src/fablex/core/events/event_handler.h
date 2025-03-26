#pragma once

#include "event.h"
#include <string>
#include <functional>

namespace fe
{

template<typename EventType>
using EventDelegate = std::function<void(const EventType& event)>;

class IEventHandler
{
public:
    virtual ~IEventHandler() { }

    void execute(const IEvent& event)
    {
        internal_execute(event);
    }

    virtual std::string get_type() = 0;

protected:
    virtual void internal_execute(const IEvent& event) = 0;
};

template<typename EventType>
class EventHandler : public IEventHandler
{
public:
    explicit EventHandler(const EventDelegate<EventType>& eventDelegate)
        : m_eventDelegate(eventDelegate), m_type(m_eventDelegate.target_type().name()) { }

    virtual std::string get_type() override
    {
        return m_type;
    }

private:
    EventDelegate<EventType> m_eventDelegate;
    std::string m_type;

    virtual void internal_execute(const IEvent& event) override
    {
        if (EventType::get_type_id_static() == event.get_type_id())
            m_eventDelegate(static_cast<const EventType&>(event));
    }
};

}