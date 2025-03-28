#pragma once

#include "types.h"
#include <functional>

namespace fe
{

class UUID
{
public:
    UUID();
    UUID(uint64 uuid);

    operator uint64() const { return m_uuid; }
    bool operator==(const UUID& uuid) const
    {
        return m_uuid == uuid.m_uuid;
    }
private:
    uint64 m_uuid;
};

}

namespace std
{

template<>
struct hash<fe::UUID>
{
    std::size_t operator()(const fe::UUID& uuid) const
    {
        return hash<uint64>()((uint64)uuid);
    }
};

}

