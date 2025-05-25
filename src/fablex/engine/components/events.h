#pragma once

#include "asset_manager/fwd.h"
#include "core/event.h"

namespace fe::engine
{

class Entity;
class MaterialComponent;

class ModelComponentUpdatedEvent : public IEvent
{
public:
    FE_DECLARE_EVENT(ModelComponentUpdatedEvent);

    ModelComponentUpdatedEvent(Entity* entity, asset::Model* oldModel, asset::Model* currModel)
        : m_entity(entity), m_oldModel(oldModel), m_currModel(currModel) { }

    Entity* entity() const { return m_entity; }
    asset::Model* old_model() const { return m_oldModel; }
    asset::Model* curr_model() const  { return m_currModel; }

private:
    Entity* m_entity;
    asset::Model* m_oldModel;
    asset::Model* m_currModel;
};

class MaterialUpdatedEvent : public IEvent
{
public:
    FE_DECLARE_EVENT(MaterialSelectedEvent);

    MaterialUpdatedEvent(MaterialComponent* component) : m_component(component) { }

    MaterialComponent* material_component() const { return m_component; }

private:
    MaterialComponent* m_component;
};

}