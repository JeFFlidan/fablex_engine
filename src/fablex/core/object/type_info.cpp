#include "type_info.h"
#include "property.h"

namespace fe
{

TypeInfo::TypeInfo(
    const char* name,
    AllocatorHandler allocatorHandler,
    uint64 size,
    uint64 alignment,
    const TypeInfo* baseTypeInfo
)
{
    m_name = name;
    m_allocatorHandler = allocatorHandler;
    m_classSize = size;
    m_classAlignment = alignment;
    m_baseTypeInfo = baseTypeInfo;
}

bool TypeInfo::is_a(const TypeInfo* typeInfo) const
{
    FE_CHECK(typeInfo);

    for (const TypeInfo* it = this; it != nullptr; it = it->m_baseTypeInfo)
        if (it->get_name_hash() == typeInfo->get_name_hash())
            return true;

    return false;
}

bool TypeInfo::is_exactly(const TypeInfo* typeInfo) const
{
    FE_CHECK(typeInfo);
    return typeInfo->get_name_hash() == get_name_hash();
}

Property* TypeInfo::get_property(const char* propertyName) const
{
    for (Property* property : m_properties)
        if (strcmp(propertyName, property->get_name().c_str()) == 0)
            return property;

    return nullptr;
}

Property* TypeInfo::get_property(Name propertyName) const
{
    return get_property(propertyName.to_string().c_str());
}

void TypeInfo::add_property(Property* property)
{
    if (get_property(property->get_name().c_str()) == nullptr)
        m_properties.push_back(property);
}

void add_property(TypeInfo* typeInfo, Property* property)
{
    FE_CHECK(typeInfo);
    FE_CHECK(property);

    typeInfo->add_property(property);
}

}