#include "entity_manager.h"
#include "events.h"

namespace fe::engine
{

void EntityManager::update()
{
    for (Entity* entity : m_entitiesToDestroy)
    {
        auto it = std::find(m_entities.begin(), m_entities.end(), entity);
        if (it != m_entities.end())
        {
            m_entities.erase(it);
            m_allocator.free(entity);
        }
    }

    m_entitiesToDestroy.clear();

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
    FE_CHECK_MSG(0, "Entity derived types are not currently supported!");
    return nullptr;
}

void EntityManager::remove_entity(Entity* entity)
{
    m_entitiesToDestroy.push_back(entity);
}

}