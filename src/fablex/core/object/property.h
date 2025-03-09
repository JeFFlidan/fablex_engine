#pragma once

#include "object.h"
#include "core/types.h"
#include "core/math.h"
#include "core/utils.h"
#include "core/macro.h"
#include <string>
#include <typeinfo>

namespace fe
{

enum class PropertyType
{
    BOOL,
    INTEGER,
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    FLOAT3X4,
    FLOAT4X4,
    STRING
};

class Property
{
public:
    virtual ~Property() = default;

    virtual PropertyType get_type() const = 0;
    virtual const char* get_name() const = 0;
    virtual uint64 get_offset() const = 0;

    template<typename T>
    void get_value(Object* object, T& outValue)
    {
        void* data = object + get_offset();
        outValue = *reinterpret_cast<T*>(data);
    }

    template<typename T>
    void get_value(Object* object, T*& outValue)
    {
        void* data = object + get_offset();
        outValue = reinterpret_cast<T*>(data);
    }
};

template<typename T, PropertyType Type>
class PropertyImpl : public Property
{
public:
    PropertyImpl(const char* name, uint64 offset) 
        : m_name(name), m_offset(offset) { }
    ~PropertyImpl() { m_name = nullptr; }

    virtual PropertyType get_type() const override { return Type; }
    virtual const char* get_name() const override { return m_name; }
    virtual uint64 get_offset() const override { return m_offset; }

private:
    const char* m_name = nullptr;
    uint64 m_offset = 0;
};

using PropertyBool = PropertyImpl<int32, PropertyType::BOOL>;
using PropertyInt = PropertyImpl<int32, PropertyType::INTEGER>;
using PropertyFloat = PropertyImpl<float, PropertyType::FLOAT>;
using PropertyFloat2 = PropertyImpl<math::Float2, PropertyType::FLOAT2>;
using PropertyFloat3 = PropertyImpl<math::Float3, PropertyType::FLOAT3>;
using PropertyFloat4 = PropertyImpl<math::Float4, PropertyType::FLOAT4>;
using PropertyFloat3x4 = PropertyImpl<math::Float3x4, PropertyType::FLOAT3X4>;
using PropertyFloat4x4 = PropertyImpl<math::Float4x4, PropertyType::FLOAT4X4>;
using PropertyString = PropertyImpl<std::string, PropertyType::STRING>;

template<typename T>
Property* allocate_property(const char* name, uint64 offset)
{
    using SupportedTypes = std::tuple<
        bool, 
        int32, 
        float, 
        math::Float2, 
        math::Float3,
        math::Float4,
        math::Float3x4,
        math::Float4x4,
        std::string
    >;

    FE_COMPILE_CHECK((TupleHasTypeV<T, SupportedTypes>));

    if constexpr (typeid(T) == typeid(bool))
        return memory_new<PropertyBool>(name, offset);
    if constexpr (typeid(T) == typeid(int32))
        return memory_new<PropertyInt>(name, offset);
    if constexpr (typeid(T) == typeid(float))
        return memory_new<PropertyFloat>(name, offset);
    if constexpr (typeid(T) == typeid(math::Float2))
        return memory_new<PropertyFloat2>(name, offset);
    if constexpr (typeid(T) == typeid(math::Float3))
        return memory_new<PropertyFloat3>(name, offset);
    if constexpr (typeid(T) == typeid(math::Float4))
        return memory_new<PropertyFloat4>(name, offset);
    if constexpr (typeid(T) == typeid(math::Float3x4))
        return memory_new<PropertyFloat3x4>(name, offset);
    if constexpr (typeid(T) == typeid(math::Float4x4))
        return memory_new<PropertyFloat4x4>(name, offset);
    if constexpr (typeid(T) == typeid(std::string))
        return memory_new<PropertyString>(name, offset);
}

}