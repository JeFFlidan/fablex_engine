#include "event_manager.h"

namespace fe
{

void EventManager::unsubscribe(uint64_t eventID, const std::string& eventHandlerTypeName)
{
    std::scoped_lock<std::mutex> locker(s_handlersByEventIDMutex);
    auto it = s_handlersByEventID.find(eventID);

    if (it == s_handlersByEventID.end())
    {
        FE_LOG(LogEvents, ERROR, "EventManager::unsubscribe(): EventManager doesn't know event with ID {}", eventID);
        return;
    }

    auto& handlers = it->second;

    for (auto beginIt = handlers.begin(); beginIt != handlers.end(); ++beginIt)
    {
        if (beginIt->get()->get_type() == eventHandlerTypeName)
        {
            handlers.erase(beginIt);
            return;
        }
    }
}

void EventManager::trigger_event(const IEvent& event)
{
    std::scoped_lock<std::mutex> locker(s_eventQueueMutex);

    auto it = s_handlersByEventID.find(event.get_type_id());
    if (it == s_handlersByEventID.end())
        return;

    for (auto& handler : it->second)
        handler->execute(event);
}

void EventManager::dispatch_events()
{
    while (!s_eventsQueue.empty())
    {
        IEvent* event = s_eventsQueue.front().get();
        auto it = s_handlersByEventID.find(event->get_type_id());
        if (it == s_handlersByEventID.end())
        {
            s_eventsQueue.pop();
            continue;
        }
        
        auto& handlers = it->second;
        for (auto& handler : handlers)
        {
            handler->execute(*event);
        }

        s_eventsQueue.pop();
    }
}    

}
