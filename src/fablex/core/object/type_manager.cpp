#include "type_manager.h"
#include "object.h"
#include "type_info.h"
#include "core/macro.h"

namespace fe
{

void TypeManager::cleanup()
{
    for (const TypeInfo* typeInfo : s_typeInfos)
        typeInfo->cleanup_properties();
}

const TypeInfo* TypeManager::get_type_info(const char* typeName)
{
    auto it = s_indexByTypeName.find(typeName);
    if (it == s_indexByTypeName.end())
        return nullptr;
    return s_typeInfos.at(it->second);
}

const TypeInfo* TypeManager::get_type_info(Name typeName)
{
    auto it = s_indexByTypeName.find(typeName.to_string());
    if (it == s_indexByTypeName.end())
        return nullptr;
    return s_typeInfos.at(it->second);
}

Object* TypeManager::create_object(const TypeInfo* typeInfo)
{
    FE_CHECK(typeInfo);
    return typeInfo->get_allocator_handler()();
}

Object* TypeManager::create_object_by_name(const char* typeName)
{
    const TypeInfo* typeInfo = get_type_info(typeName);
    FE_CHECK(typeInfo);
    return typeInfo->get_allocator_handler()();
}

void TypeManager::register_type(const TypeInfo* typeInfo)
{
    FE_CHECK(typeInfo);

    auto it = s_indexByTypeName.find(typeInfo->get_str_name());
    if (it != s_indexByTypeName.end())
        return;

    s_typeInfos.push_back(typeInfo);
    s_indexByTypeName[typeInfo->get_str_name()] = (uint32)s_typeInfos.size() - 1;
}

}