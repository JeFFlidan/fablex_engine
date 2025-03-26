#pragma once

#include "event_handler.h"
#include "core/logger.h"

#include <mutex>
#include <memory>
#include <queue>
#include <vector>
#include <unordered_map>

namespace fe
{

FE_DEFINE_LOG_CATEGORY(LogEvents);

class EventManager
{
    public:
        template<typename EventType>
        static void subscribe(uint64 eventID, const EventDelegate<EventType>& eventDelegate)
        {
            std::scoped_lock<std::mutex> locker(s_handlersByEventIDMutex);

            auto it = s_handlersByEventID.find(eventID);
            std::unique_ptr<EventHandler<EventType>> eventHandler(new EventHandler<EventType>(eventDelegate));

            if (it != s_handlersByEventID.end())
            {
                for (auto& handler : s_handlersByEventID[eventID])
                {
                    if (handler->get_type() == eventHandler->get_type())
                    {
                        FE_LOG(LogEvents, ERROR, "EventManager::subscribe(): Engine subscribed the event to the same event handler.");
                        return;
                    }
                }
            }

            s_handlersByEventID[eventID].emplace_back(std::move(eventHandler));
        }

        template<typename EventType>
        static void subscribe(const EventDelegate<EventType>& eventDelegate)
        {
            subscribe(EventType::get_type_id_static(), eventDelegate);
        }
    
        static void unsubscribe(uint64 eventID, const std::string& eventHandlerTypeName);

        template<typename CustomEvent>
        static void enqueue_event(const CustomEvent& event)
        {
            std::scoped_lock<std::mutex> locker(s_eventQueueMutex);
            s_eventsQueue.emplace(new CustomEvent(event));
        }
    
        static void trigger_event(const IEvent& event);
        static void dispatch_events();
    
    private:
        using EventHandlerArray = std::vector<std::unique_ptr<IEventHandler>>;

        inline static std::queue<std::unique_ptr<IEvent>> s_eventsQueue{};
        inline static std::unordered_map<uint64, EventHandlerArray> s_handlersByEventID{};
        
        inline static std::mutex s_eventQueueMutex{};
        inline static std::mutex s_handlersByEventIDMutex{};
};
    
}