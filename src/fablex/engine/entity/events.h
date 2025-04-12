#pragma once

#include "core/event.h"

namespace fe::engine
{

class Entity;

class EntityCreatedEvent : public IEvent
{
public:
    FE_DECLARE_EVENT(EntityCreatedEvent);

    EntityCreatedEvent(Entity* entity) : m_entity(entity) { }

    Entity* get_entity() const { return m_entity; }

private:
    Entity* m_entity = nullptr;
};

}