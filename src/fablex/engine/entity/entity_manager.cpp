#include "entity_manager.h"
#include "events.h"

namespace fe::engine
{

void EntityManager::update()
{
    for (Entity* entity : m_entitiesToRemove)
    {
        auto it = std::find(m_entities.begin(), m_entities.end(), entity);
        if (it != m_entities.end())
        {
            EventManager::trigger_event(EntityRemovedEvent(entity));
            
            m_entities.erase(it);
            m_allocator.free(entity);
        }
    }

    m_entitiesToRemove.clear();

    for (Entity* entity : m_entitiesToCreate)
    {
        EventManager::enqueue_event(EntityCreatedEvent(entity));
        m_entities.push_back(entity);
    }

    m_entitiesToCreate.clear();
}

Entity* EntityManager::create_entity()
{
    m_entitiesToCreate.push_back(m_allocator.allocate());
    return m_entitiesToCreate.back();
}

Entity* EntityManager::create_entity(const TypeInfo* typeInfo)
{
    if (typeInfo->is_exactly(Entity::get_static_type_info()))
        return create_entity();

    Entity* entity = static_cast<Entity*>(TypeManager::create_object(typeInfo));
    m_entitiesToCreate.push_back(entity);

    return entity;
}

void EntityManager::remove_entity(Entity* entity)
{
    m_entitiesToRemove.push_back(entity);
}

}