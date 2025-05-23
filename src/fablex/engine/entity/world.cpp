#include "world.h"
#include "engine/components/camera_component.h"
#include "core/timer.h"

namespace fe::engine
{

FE_DEFINE_OBJECT(World, Object);

Entity* World::create_entity()
{
    engine::Entity* entity = m_entityManager.create_entity();
    entity->on_world_set(this);
    return entity;
}

Entity* World::create_entity(const TypeInfo* typeInfo)
{
    engine::Entity* entity = m_entityManager.create_entity(typeInfo);
    entity->on_world_set(this);
    return entity;
}

void World::remove_entity(Entity* entity)
{
    m_entityManager.remove_entity(entity);
}

void World::update_pre_entities_update()
{
    m_entityManager.update();

    const std::vector<Entity*>& entities = m_entityManager.get_entities();
    for (Entity* entity : entities)
    {
        entity->update_world_transform();
    }
}

void World::update_camera_entities()
{
    const std::vector<Entity*>& entities = m_entityManager.get_entities();
    for (Entity* entity : entities)
        if (CameraComponent* cameraComponent = entity->get_component<CameraComponent>())
            cameraComponent->update(Timer::get_delta_time());
}

}