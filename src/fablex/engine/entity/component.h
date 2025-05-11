#pragma once

#include "core/object.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::engine
{

class World;
class Entity;

class Component : public Object
{
    FE_DECLARE_OBJECT(Component);
    FE_DECLARE_PROPERTY_REGISTER(Entity);

public:
    virtual void update(float deltaTime) { }

    void set_active(bool active) { m_active = active; }
    bool is_active() const { return m_active; }

    virtual void on_init() { }
    virtual void on_entity_set(Entity* entity) { m_entity = entity; }
    virtual void on_world_set(World* world) { m_world = world; }
    
    virtual void on_cleanup() { }
    virtual void on_entity_remove() { m_entity = nullptr; }
    virtual void on_world_remove() { m_world = nullptr; }

    virtual void fill_shader_data(ShaderEntity& outShaderEntity) const { }

    Entity* get_entity() const { return m_entity; }
    World* get_world() const { return m_world; }

protected:
    Entity* m_entity = nullptr;
    World* m_world = nullptr;

    bool m_active = false;
};

}