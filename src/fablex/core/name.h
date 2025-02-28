#pragma once

#include "types.h"
#include "fmt/format.h"
#include <string>

namespace fe
{

class Name
{
public:
    using ID = uint32;
    static constexpr ID s_invalidID = ~0u;

    Name();
    Name(const std::string& strName);
    Name(const char* strName);
    explicit Name(ID id);
    
    Name(const Name& other);
    Name(Name&& other);

    Name& operator=(const Name& other);
    Name& operator=(Name&& other);

    bool operator==(const Name& other) const;
    bool operator<(const Name& other) const;

    const std::string& to_string() const;
    ID to_id() const;

    bool is_valid() const;

private:
    ID m_id;
};

}

namespace std
{
    template<>
    struct hash<fe::Name>
    {
        size_t operator()(const fe::Name& key) const
        {
            return key.to_id();
        }
    };
}

template<>
struct fmt::formatter<fe::Name>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::Name& name, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "{0}", name.to_string());
    }
};
