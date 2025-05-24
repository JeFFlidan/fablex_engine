#include "entity.h"
#include "world.h"
#include "component.h"
#include "core/file_system/archive.h"

namespace fe::engine
{

FE_DEFINE_OBJECT(Entity, Object)

FE_BEGIN_PROPERTY_REGISTER(Entity)
{
    FE_REGISTER_PROPERTY(Entity, m_worldTransform);
    FE_REGISTER_PROPERTY(Entity, m_position, EditAnywhere());
    FE_REGISTER_PROPERTY(Entity, m_rotation, EditAnywhere());
    FE_REGISTER_PROPERTY(Entity, m_scale, EditAnywhere());
}
FE_END_PROPERTY_REGISTER(Entity)

Entity::Entity()
    : m_worldTransform(IDENTITY_MATRIX),
    m_prevWorldTransform(IDENTITY_MATRIX),
    m_position(0, 0, 0),
    m_rotation(Quat::identity()),
    m_scale(1, 1, 1)
{
    
}

Entity::~Entity()
{
    for (Component* component : m_components)
        memory_delete(component);

    for (Entity* entity : m_children)
        m_world->remove_entity(entity);

    m_components.clear();
    m_children.clear();
    m_world = nullptr;
    m_rootEntity = nullptr;
}

Entity* Entity::create_child()
{
    FE_CHECK(m_world);
    Entity* child = m_children.emplace_back(m_world->create_entity());
    child->set_root(this);
    return child;
}

Entity* Entity::create_child(const TypeInfo* typeInfo)
{
    FE_CHECK(m_world);
    FE_CHECK(typeInfo);

    Entity* child = m_children.emplace_back(m_world->create_entity(typeInfo));
    child->set_root(this);
    return child;
}

Component* Entity::create_component(const TypeInfo* typeInfo)
{
    // TODO: Think how to allocate component from pool
    FE_CHECK(typeInfo);

    Component* component = static_cast<Component*>(TypeManager::create_object(typeInfo));
    m_components.push_back(component);
    if (m_world) component->on_world_set(m_world);
    component->on_entity_set(this);

    return component;
}

Component* Entity::get_component(const TypeInfo* typeInfo) const
{
    for (Component* component : m_components)
        if (component->is_a(typeInfo))
            return component;

    return nullptr;
}

void Entity::update_world_transform()
{
    m_prevWorldTransform = m_worldTransform;

    Matrix worldTransformMat = Matrix::scaling(m_scale) * Matrix::rotation(m_rotation) * Matrix::translation(m_position);

    if (m_rootEntity)
        worldTransformMat *= m_rootEntity->get_world_transform().to_matrix();

    m_worldTransform = worldTransformMat;
}

Float3 Entity::get_world_position() const
{
    if (m_rootEntity)
    {
        m_rootEntity->update_world_transform();
        return Vector3::transform(m_position, m_rootEntity->get_world_transform());
    }

    return m_position;
}

Float4x4 Entity::get_local_transform() const
{
    return Matrix::scaling(m_scale) * Matrix::rotation(m_rotation) * Matrix::translation(m_position);
}

void Entity::translate(const Float3& deltaPosition)
{
    m_position.x += deltaPosition.x;
    m_position.y += deltaPosition.y;
    m_position.z += deltaPosition.z;
}

void Entity::set_rotation(const Float3& eulerAngles, AngleUnit angleUnit)
{
    // Take from WickedEngine. Maybe, I will change this in the future
    Quat x = Quat::rotation(eulerAngles.x, 0.0f, 0.0f, angleUnit);
    Quat y = Quat::rotation(0.0f, eulerAngles.y, 0.0f, angleUnit);
    Quat z = Quat::rotation(0.0f, 0.0f, eulerAngles.z, angleUnit);

    m_rotation = Quat::multiply(x, m_rotation);
    m_rotation = Quat::multiply(m_rotation, y);
    m_rotation = Quat::multiply(z, m_rotation);
    m_rotation.normalize();

    // m_rotation = Quat::rotation(eulerAngles.x, eulerAngles.y, eulerAngles.z, angleUnit);
}

void Entity::set_rotation(const Float3& axis, float angle, AngleUnit angleUnit)
{
    m_rotation = Quat::rotation_axis(axis, angle, angleUnit);
}

void Entity::serialize(Archive& archive) const
{
    Object::serialize(archive); 

    archive << m_name;
    archive << m_position;
    archive << Float4(m_rotation);
    archive << m_scale;

    archive << m_tags.size();
    for (uint64 tag : m_tags)
        archive << tag;

    archive << m_components.size();
    for (Component* component : m_components)
    {
        archive << component->get_type_info()->get_name();
        component->serialize(archive);
    }

    archive << m_children.size();
    for (Entity* entity : m_children)
    {
        archive << entity->get_type_info()->get_name();
        entity->serialize(archive);
    }
}

void Entity::deserialize(Archive& archive)
{
    Object::deserialize(archive);

    FE_CHECK(m_world);

    archive >> m_name;
    archive >> m_position;

    // Must add quat to archive
    Float4 rotation;
    archive >> rotation;
    m_rotation = Vector(rotation);

    archive >> m_scale;

    uint32 tagCount;
    archive >> tagCount;

    for (uint32 i = 0; i != tagCount; ++i)
    {
        uint64 tag;
        archive >> tag;
        m_tags.insert(tag); 
    }

    uint32 componentCount = 0;
    archive >> componentCount;

    for (uint32 i = 0; i != componentCount; ++i)
    {
        std::string componentTypeName;
        archive >> componentTypeName;

        const TypeInfo* typeInfo = TypeManager::get_type_info(componentTypeName);
        FE_CHECK(typeInfo);

        Component* component = create_component(typeInfo);
        component->deserialize(archive);
    }

    uint32 entityCount = 0;
    archive >> entityCount;

    for (uint32 i = 0; i != entityCount; ++i)
    {
        std::string entityTypeName;
        archive >> entityTypeName;
        const TypeInfo* typeInfo = TypeManager::get_type_info(entityTypeName);
        FE_CHECK(typeInfo);

        Entity* entity = create_child(typeInfo);
        entity->deserialize(archive);
    }
}

}