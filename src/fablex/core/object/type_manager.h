#pragma once

#include "core/name.h"
#include "core/types.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace fe
{

class TypeInfo;
class Object;

class TypeManager
{
public:
    static const TypeInfo* get_type_info(const char* typeName);
    static const TypeInfo* get_type_info(Name typeName);
    
    static Object* create_object_by_name(const char* typeName);

    static void register_type(const TypeInfo* typeInfo);

private:
    inline static std::unordered_map<std::string, uint32> s_indexByTypeName{};
    inline static std::vector<const TypeInfo*> s_typeInfos{};
};

}