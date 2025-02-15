#pragma once

#include "types.h"
#include <string>
#include <vector>
#include "unordered_map"

namespace fe
{

class NameRegistry
{
public:
    static uint32 to_id(const std::string& strName);
    static const std::string& to_string(uint32 id);

private:
    inline static std::unordered_map<std::string, uint32> s_nameToId{};
    inline static std::vector<std::string> s_idToName{};
};

}