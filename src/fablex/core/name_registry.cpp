#include "name_registry.h"

namespace fe
{

uint32 NameRegistry::to_id(const std::string& strName)
{
    auto it = s_nameToId.find(strName);
    if (it != s_nameToId.end())
    {
        return it->second;
    }

    s_idToName.push_back(strName);
    s_nameToId[strName] = s_idToName.size() - 1;

    return (uint32)s_idToName.size() - 1;
}

const std::string& NameRegistry::to_string(uint32 id)
{
    return s_idToName.at(id);
}

}