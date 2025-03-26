#pragma once

#include "core/name.h"
#include "core/types.h"
#include <functional>

namespace fe
{

class Property;
class Object;
class TypeInfo;
class TypeManager;

void add_property(TypeInfo* typeInfo, Property* property);

class TypeInfo
{
public:
    using AllocatorHandler = std::function<Object*()>;

    friend TypeManager;
    friend void add_property(TypeInfo* typeInfo, Property* property);

    TypeInfo(
        const char* name,
        AllocatorHandler allocatorHandler,
        uint64 size,
        uint64 alignment,
        const TypeInfo* baseTypeInfo
    );

    const char* get_str_name() const { return m_name; };
    Name get_name() const { return m_name; }
    uint64 get_name_hash() const { return m_nameHash; }
    
    uint64 get_class_size() const { return m_classSize; }
    uint64 get_class_alignment() const { return m_classAlignment; }
    AllocatorHandler get_allocator_handler() const { return m_allocatorHandler; }

    bool is_a(const TypeInfo* typeInfo) const;
    bool is_exactly(const TypeInfo* typeInfo) const;

    template<typename T>
    bool is_a() const
    {
        return is_a(T::get_static_type_info());
    }

    template<typename T>
    bool is_exactly() const
    {
        return is_exactly(T::get_static_type_info());
    }

    Property* get_property(const char* propertyName) const;
    Property* get_property(Name propertyName) const;
    const std::vector<Property*>& get_properties() const { return m_properties; }

protected:
    const char* m_name;
    AllocatorHandler m_allocatorHandler;
    uint64 m_classSize;
    uint64 m_classAlignment;
    
    const TypeInfo* m_baseTypeInfo = nullptr;
    uint64 m_nameHash;

    mutable std::vector<Property*> m_properties;

    void add_property(Property* property);
    void cleanup_properties() const;
};

}