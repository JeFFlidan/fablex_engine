#pragma once

#include "core/compile_time_hash.h"
#include "core/event.h"

namespace fe::engine
{

class Entity;

template<typename TagType>
class TagAddedEvent { };

#define FE_DEFINE_ENTITY_TAG_ADDED_EVENT(TagType)                   \
    template<>                                                      \
    class TagAddedEvent<TagType> : public fe::IEvent          \
    {                                                               \
    public:                                                         \
        FE_DECLARE_EVENT(TagAddedEvent##TagType);             \
        TagAddedEvent(Entity* entity) : m_entity(entity) { }  \
        Entity* get_entity() const { return m_entity; }             \
    private:                                                        \
        Entity* m_entity;                                           \
    };

#define FE_TYPE_TAG_HASH(x) fe::compile_time_fnv1(#x)

#define FE_DEFINE_TAG(TagName)                      \
    struct TagName                                  \
    {                                               \
        enum Type : uint64                          \
        {                                           \
            TYPE_ID = FE_TYPE_TAG_HASH(TagName)     \
        };                                          \
    };                                              \
    FE_DEFINE_ENTITY_TAG_ADDED_EVENT(TagName)


FE_DEFINE_TAG(EditorCameraTag);

}
