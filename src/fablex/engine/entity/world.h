#pragma once

#include "entity_manager.h"

namespace fe::engine
{

// TODO: Think about saving components in separate arrays to make iteration easier
class World : public Object
{
    FE_DECLARE_OBJECT(World);

public:
    Entity* create_entity();
    Entity* create_entity(const TypeInfo* typeInfo);

    template<typename T>
    T* create_child()
    {
        FE_COMPILE_CHECK((std::is_base_of_v<Entity, T>));
        return static_cast<T*>(create_entity(T::get_static_type_info()));
    }

    void remove_entity(Entity* entity);

    void update_pre_entities_update();
    void update_camera_entities();

    EntityManager& get_entity_manager() { return m_entityManager; }
    const EntityManager& get_entity_manager() const { return m_entityManager; }

private:
    EntityManager m_entityManager;
};

}