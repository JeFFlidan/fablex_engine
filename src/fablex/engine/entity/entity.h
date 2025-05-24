#pragma once

#include "core/object.h"
#include "tags.h"
#include <unordered_set>

namespace fe::engine
{

class World;
class Component;

class Entity : public Object
{
    FE_DECLARE_OBJECT(Entity);
    FE_DECLARE_PROPERTY_REGISTER(Entity);

public:
    Entity();
    ~Entity();

    void set_name(const std::string& name) { m_name = name; }
    const std::string& get_name() const { return m_name;}

    virtual void on_world_set(World* world) { m_world = world; }

    Entity* create_child();
    Entity* create_child(const TypeInfo* typeInfo);

    template<typename T>
    T* create_child()
    {
        FE_COMPILE_CHECK((std::is_base_of_v<Entity, T>));
        return static_cast<T*>(create_child(T::get_static_type_info()));
    }

    const std::vector<engine::Entity*>& get_children() const { return m_children; }

    Component* create_component(const TypeInfo* typeInfo);
    
    template<typename T>
    T* create_component()
    {
        FE_COMPILE_CHECK((std::is_base_of_v<Component, T>));
        return static_cast<T*>(create_component(T::get_static_type_info()));
    }

    const std::vector<Component*>& get_components() const { return m_components; }
    Component* get_component(const TypeInfo* typeInfo) const;

    template<typename T>
    T* get_component() const
    {
        FE_COMPILE_CHECK((std::is_base_of_v<Component, T>));
        return static_cast<T*>(get_component(T::get_static_type_info()));
    }

    void set_root(Entity* entity) { m_rootEntity = entity; }
    Entity* get_root() const { return m_rootEntity; }

    Float3 get_position() const { return m_position; }
    Quat get_rotation() const { return m_rotation; }
    Float3 get_euler_rotation() const { return m_rotation.to_euler(); }
    Float3 get_scale() const { return m_scale; }

    void update_world_transform();

    Float3 get_world_position() const;
    Float4x4 get_local_transform() const;
    const Float4x4& get_world_transform() const { return m_worldTransform; }
    const Float4x4& get_prev_world_transform() const { return m_prevWorldTransform; }

    void translate(const Float3& deltaPosition);
    void set_position(const Float3& position) { m_position = position; }
    void set_scale(const Float3& scale) { m_scale = scale; }
    void set_rotation(const Quat& rotation) { m_rotation = rotation; }
    void set_rotation(const Float3& eulerAngles, AngleUnit angleUnit = AngleUnit::DEGREES);

    // Quaternion rotation
    void set_rotation(const Float3& axis, float angle, AngleUnit = AngleUnit::DEGREES);

    template<typename TagType>
    void add_tag()
    {
        m_tags.insert(TagType::TYPE_ID);
        EventManager::enqueue_event(TagAddedEvent<TagType>());
    }

    template<typename TagType>
    bool has_tag()
    {
        auto it = m_tags.find(TagType::TYPE_ID);
        if (it != m_tags.end())
            return true;
        return false;
    }

    virtual void serialize(Archive& archive) const override;
    virtual void deserialize(Archive& archive) override;

private:
    std::string m_name = "undefined";

    std::vector<Component*> m_components;
    std::vector<Entity*> m_children;
    std::unordered_set<uint64> m_tags;

    World* m_world = nullptr;
    Entity* m_rootEntity = nullptr;
    
    Float4x4 m_worldTransform;
    Float4x4 m_prevWorldTransform;

    Float3 m_position;
    Quat m_rotation;
    Float3 m_scale;
};

}