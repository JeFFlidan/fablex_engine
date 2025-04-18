#pragma once

#include "config.h"
#include "entity.h"
#include "core/pool_allocator.h"

namespace fe::engine
{

class EntityManager
{
public:
    void update();

    Entity* create_entity();
    Entity* create_entity(const TypeInfo* typeInfo);

    template<typename T>
    T* create_entity()
    {
        FE_COMPILE_CHECK((std::is_base_of_v<Entity, T>));
        return static_cast<T*>(create_entity(T::get_static_type_info()));
    }

    void remove_entity(Entity* entity);
    
    const std::vector<Entity*>& get_entities() const { return m_entities; }

private:
    ThreadSafePoolAllocator<Entity, ENTITY_POOL_SIZE> m_allocator;
    
    std::vector<Entity*> m_entities;
    std::vector<Entity*> m_entitiesToCreate;
    std::vector<Entity*> m_entitiesToDestroy;
};

}