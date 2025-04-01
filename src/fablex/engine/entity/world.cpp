#include "world.h"

namespace fe::engine
{

FE_DEFINE_OBJECT(World, Object);

void World::update()
{
    m_entityManager.update();
}

Entity* World::create_entity()
{
    return m_entityManager.create_entity();
}

Entity* World::create_entity(const TypeInfo* typeInfo)
{
    return m_entityManager.create_entity(typeInfo);
}

void World::remove_entity(Entity* entity)
{
    m_entityManager.remove_entity(entity);
}

}