#pragma once

#include "sparse_set.h"
#include "core/object/object.h"
#include "core/memory/pool_allocator.h"

namespace fe::engine
{

template<typename T>
struct ComponentPoolSize
{
    static constexpr uint64 poolSize = 64;
};

#define FE_DEFINE_COMPONENT_POOL_SIZE(ComponentType, PoolSize)  \
    template<>                                                  \
    struct ComponentPoolSize                                    \
    {                                                           \
        static constexpr uint64 poolSize = PoolSize;            \
    };

#define FE_DECLARE_COMPONENT(ComponentType)                                                             \
    using PoolAllocatorType = PoolAllocator<ComponentType, ComponentPoolSize<ComponentType>::poolSize>; \
    FE_DECLARE_OBJECT_CUSTOM_ALLOCATOR(ComponentType, PoolAllocatorType)                                \
    FE_DECLARE_PROPERTY_REGISTER(ComponentType)                                                         \
    using ForEachHandler = SparseSet<ComponentType>::ForEachHandler;                                    \
    inline static SparseSet<ComponentType> s_sparseSet;                                                 \
    static void for_each(const ForEachHandler& handler);                                                \
    static uint32 count();                                                                              \
    virtual ~ComponentType() override;                                                                  \
    virtual void on_entity_set(Entity* entity) override;                                                \


#define FE_DEFINE_COMPONENT(ComponentType, ParentComponentType)  \
    FE_DEFINE_OBJECT(ComponentType, ParentComponentType)         \
    ComponentType::~ComponentType()                              \
    {                                                            \
        s_sparseSet.erase(m_entity);                             \
    }                                                            \
    void ComponentType::for_each(const ForEachHandler& handler)  \
    {                                                            \
        s_sparseSet.for_each(handler);                           \
    }                                                            \
    uint32 ComponentType::count()                                \
    {                                                            \
        return s_sparseSet.size();                               \
    }                                                            \
    void ComponentType::on_entity_set(Entity* entity)            \
    {                                                            \
        ParentComponentType::on_entity_set(entity);              \
        s_sparseSet.insert(*entity, this);                       \
    }

}