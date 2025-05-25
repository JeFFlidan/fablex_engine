#pragma once

#include "entity/fwd.h"
#include "core/event.h"

namespace fe::engine
{

class ModelEntityCreationRequest : public IEvent
{
public:
    FE_DECLARE_EVENT(ModelCreationRequest);
};

class PointLightEntityCreationRequest : public IEvent
{
public:
    FE_DECLARE_EVENT(PointLightEntityCreationRequest);
};

class EntityRemovalRequest : public IEvent
{
public:
    FE_DECLARE_EVENT(EntityRemovalRequest);

    EntityRemovalRequest(Entity* entity) : m_entity(entity) { }

    Entity* entity() const { return m_entity; }

private:
    Entity* m_entity;
};

class ProjectSavingRequest : public IEvent
{
public:
    FE_DECLARE_EVENT(ProjectSavingRequest);
};

}