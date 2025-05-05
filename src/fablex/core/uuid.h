#pragma once

#include "types.h"
#include <functional>
#include <fmt/format.h>

namespace fe
{

class UUID
{
public:
    static const UUID INVALID;

    UUID();
    UUID(uint64 uuid);

    operator uint64() const { return m_uuid; }
    
    bool operator==(const UUID& uuid) const
    {
        return m_uuid == uuid.m_uuid;
    }

    bool operator!=(const UUID& other) const
    {
        return m_uuid != other.m_uuid;
    }

private:
    uint64 m_uuid;
};

inline const UUID UUID::INVALID = 0;

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

template<>
struct fmt::formatter<fe::UUID>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::UUID& uuid, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", (uint64)uuid);
    }
};

