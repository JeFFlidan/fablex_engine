#pragma once

#include "name.h"
#include "json.hpp"
#include "utils.h"

namespace fe
{

constexpr const char* g_nameKey = "Name";

inline void to_json(nlohmann::json& j, const Name& name)
{
    j = name.to_string();
}

inline void from_json(const nlohmann::json& j, Name& name)
{
    name = Name(j.get<std::string>());
}

}

template<typename Enum>
struct SerializationInfo { };

#define FE_SERIALIZE_ENUM(Namespace, EnumType, ...)                                     \
    template<>                                                                          \
    struct SerializationInfo<EnumType>                                                  \
    {                                                                                   \
        static constexpr std::pair<EnumType, const char*> m[] = __VA_ARGS__;            \
        static std::string get_string(EnumType e)                                       \
        {                                                                               \
            auto it = std::find_if(std::begin(m), std::end(m),                          \
                [e](const std::pair<EnumType, const char*>& ejPair)                     \
                {                                                                       \
                    return ejPair.first == e;                                           \
                }                                                                       \
            );                                                                          \
            return ((it != std::end(m)) ? it : std::begin(m))->second;                  \
        }                                                                               \
        static EnumType get_enum(const std::string& str)                                \
        {                                                                               \
            auto it = std::find_if(std::begin(m), std::end(m),                          \
            [&str](const std::pair<EnumType, const char*>& ejPair)                      \
                {                                                                       \
                    return strcmp(ejPair.second, fe::Utils::to_upper(str).c_str()) == 0;\
                }                                                                       \
            );                                                                          \
            return ((it != std::end(m)) ? it : std::begin(m))->first;                   \
        }                                                                               \
    };                                                                                  \
    namespace Namespace {                                                               \
    template<typename BasicJsonType>                                                    \
    inline void to_json(BasicJsonType& j, EnumType e)                                   \
    {                                                                                   \
        static_assert(std::is_enum_v<EnumType>, #EnumType " must be an enum!");         \
        j = ::SerializationInfo<EnumType>::get_string(e);                               \
    }                                                                                   \
    template<typename BasicJsonType>                                                    \
    inline void from_json(const BasicJsonType& j, EnumType& e)                          \
    {                                                                                   \
        static_assert(std::is_enum_v<EnumType>, #EnumType " must be an enum!");         \
        e = ::SerializationInfo<EnumType>::get_enum(j);                                 \
    }                                                                                   \
    }                                                                                   \
    inline std::string to_string(const EnumType& e)                                     \
    {                                                                                   \
        static_assert(std::is_enum_v<EnumType>, #EnumType " must be an enum!");         \
        return ::SerializationInfo<EnumType>::get_string(e);                            \
    }                                                                                   \
    inline void to_enum(const std::string& str, EnumType& outEnum)                      \
    {                                                                                   \
        static_assert(std::is_enum_v<EnumType>, #EnumType " must be an enum!");         \
        outEnum = ::SerializationInfo<EnumType>::get_enum(str);\
    }