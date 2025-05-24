#pragma once

#include "type_info.h"
#include "type_manager.h"
#include "core/memory.h"

namespace fe
{

class Object;
class Property;
class Archive;

#define FE_DECLARE_OBJECT(TypeName)                                                                             \
    public:                                                                                                     \
        static const TypeInfo s_typeInfo;                                                                       \
        static void static_constructor(void* ptr) { reinterpret_cast<TypeName*>(ptr)->TypeName::TypeName(); }   \
        static Object* allocate() { return (Object*)memory_new<TypeName>(); }                                   \
        static const TypeInfo* get_static_type_info() { return &TypeName::s_typeInfo; }                         \
        virtual const TypeInfo* get_type_info() const { return &TypeName::s_typeInfo; }

#define FE_DEFINE_ROOT_OBJECT(TypeName)             \
    const TypeInfo TypeName::s_typeInfo(            \
        #TypeName,                                  \
        TypeName::allocate,                         \
        sizeof(TypeName),                           \
        alignof(TypeName),                          \
        nullptr                                     \
    )

#define FE_DEFINE_OBJECT(TypeName, BaseTypeName)    \
    const TypeInfo TypeName::s_typeInfo(            \
        #TypeName,                                  \
        TypeName::allocate,                         \
        sizeof(TypeName),                           \
        alignof(TypeName),                          \
        BaseTypeName::get_static_type_info()        \
    );                                              \
    static struct TypeName##Registrator             \
    {                                               \
        TypeName##Registrator()                     \
        {                                           \
            TypeManager::register_type(             \
                TypeName::get_static_type_info());  \
        }                                           \
    } g_##TypeName##Registrator;

class Object
{
    FE_DECLARE_OBJECT(Object)

public:
    Object() = default;
    virtual ~Object() = default;

    bool is_exactly(const TypeInfo* classTypeInfo) const
    {
        return get_type_info()->is_exactly(classTypeInfo);
    }

    bool is_a(const TypeInfo* classTypeInfo) const
    {
        return get_type_info()->is_a(classTypeInfo);
    }

    template<typename T>
    bool is_exactly() const
    {
        // FE_COMPILE_CHECK((std::is_base_of_v<Object, T>));    // Does not work as expected
        return is_exactly(T::get_static_type_info());
    }

    template<typename T>
    bool is_a() const
    {
        // FE_COMPILE_CHECK((std::is_base_of_v<Object, T>));    // Does not work as expected
        return is_a(T::get_static_type_info());
    }

    const std::vector<Property*>& get_properties() const
    {
        return get_type_info()->get_properties();
    }

    virtual void serialize(Archive& archive) const;
    virtual void deserialize(Archive& archive);
};

template<typename T>
T* create_object()
{
    FE_COMPILE_CHECK((std::is_base_of_v<Object, T>));
    return (T*)TypeManager::create_object_by_name(T::get_static_type_info().get_str_name());
}

inline Object* create_object(Name name)
{
    return TypeManager::create_object_by_name(name.to_string().c_str());
}

inline void destroy_object(Object* object)
{
    memory_delete(object);
}

}