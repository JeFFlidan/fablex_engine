#include "world.h"
#include "engine/components/camera_component.h"

#include "core/timer.h"
#include "core/file_system/archive.h"

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

void World::serialize(Archive& archive) const
{
    Object::serialize(archive);

    archive << m_entityManager.get_entities().size();
    for (Entity* entity : m_entityManager.get_entities())
    {
        if (entity->get_root())
            continue;

        archive << entity->get_type_info()->get_name();
        entity->serialize(archive);
    }
}

void World::deserialize(Archive& archive)
{
    Object::deserialize(archive);

    uint32 entityCount = 0;
    archive >> entityCount;

    for (uint32 i = 0; i != entityCount; ++i)
    {
        std::string entityTypeName;
        archive >> entityTypeName;

        const TypeInfo* typeInfo = TypeManager::get_type_info(entityTypeName);
        FE_CHECK(typeInfo);

        Entity* entity = create_entity(typeInfo);
        entity->on_world_set(this);

        entity->deserialize(archive);
    }
}

}