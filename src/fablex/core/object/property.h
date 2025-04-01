#pragma once

#include "object.h"
#include "attribute.h"
#include "core/types.h"
#include "core/math.h"
#include "core/macro.h"
#include <string>
#include <utility>

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
    QUAT,
    STRING
};

template<typename T>
struct PropertyTypeEnumMapper {};

#define FE_DEFINE_PROPERTY_TYPE_ENUM_MAPPER(Type, PropertyTypeEnum)             \
    template<>                                                                  \
    struct PropertyTypeEnumMapper<Type>                                         \
    {                                                                           \
        constexpr static PropertyType value = PropertyType::PropertyTypeEnum;   \
    };

FE_DEFINE_PROPERTY_TYPE_ENUM_MAPPER(bool, BOOL);
FE_DEFINE_PROPERTY_TYPE_ENUM_MAPPER(int32, INTEGER);
FE_DEFINE_PROPERTY_TYPE_ENUM_MAPPER(float, FLOAT);
FE_DEFINE_PROPERTY_TYPE_ENUM_MAPPER(Float2, FLOAT2);
FE_DEFINE_PROPERTY_TYPE_ENUM_MAPPER(Float3, FLOAT3);
FE_DEFINE_PROPERTY_TYPE_ENUM_MAPPER(Float4, FLOAT4);
FE_DEFINE_PROPERTY_TYPE_ENUM_MAPPER(Float3x4, FLOAT3X4);
FE_DEFINE_PROPERTY_TYPE_ENUM_MAPPER(Float4x4, FLOAT4X4);
FE_DEFINE_PROPERTY_TYPE_ENUM_MAPPER(Quat, QUAT);
FE_DEFINE_PROPERTY_TYPE_ENUM_MAPPER(std::string, STRING);

class Property
{
public:
    virtual ~Property() = default;

    virtual PropertyType get_type() const = 0;
    virtual const std::string& get_name() const = 0;
    virtual uint64 get_offset() const = 0;
    virtual uint64 get_size() const = 0;

    template<typename T>
    void get_value(Object* object, T& outValue)
    {
        outValue = *reinterpret_cast<T*>(get_property_ptr(object));
    }

    template<typename T>
    void get_value(Object* object, T*& outValue)
    {
        outValue = reinterpret_cast<T*>(get_property_ptr(object));
    }

    template<typename T>
    void set_value(Object* object, const T& value)
    {
        memcpy(get_property_ptr(object), &value, sizeof(T));
    }

    template<typename T>
    bool has_attribute() const
    {
        FE_COMPILE_CHECK((std::is_base_of_v<Attribute, T>));
        return has_attribute(T::name);
    }

    template<typename T>
    const T* get_attribute() const
    {
        FE_COMPILE_CHECK((std::is_base_of_v<Attribute, T>));
        if (const void* attrValue = get_attribute(T::name))
            return static_cast<const T*>(attrValue);
        return nullptr;
    }

protected:
    virtual bool has_attribute(const char* attrName) const { return false; }
    virtual const void* get_attribute(const char* attrName) const { return nullptr; }

private:
    void* get_property_ptr(Object* object) const
    {
        FE_CHECK(object);
        return reinterpret_cast<uint8*>(object) + get_offset();
    }
};

template<typename T>
class PropertyImpl : public Property
{
public:
    PropertyImpl(const char* name)
        : m_name(name)
    {
        if (m_name.find("m_") == 0)
            m_name = m_name.substr(2);
        if (!m_name.empty())
            m_name[0] = std::toupper(m_name[0]);
    }

    virtual PropertyType get_type() const override { return PropertyTypeEnumMapper<T>::value; }
    virtual const std::string& get_name() const override { return m_name; }
    virtual uint64 get_offset() const override { return 0; };
    virtual uint64 get_size() const override { return 0; };

private:
    std::string m_name;
};

template<typename PropertyClass>
Property* allocate_property(const char* name)
{
    return memory_new<PropertyClass>(name);
}

#define FE_DECLARE_PROPERTY_REGISTER(TypeName)  \
    static void register_properties();

#define FE_BEGIN_PROPERTY_REGISTER(TypeName)    \
    void TypeName::register_properties()

#define FE_END_PROPERTY_REGISTER(TypeName)      \
    static struct TypeName##PropertyRegistrator \
    {                                           \
        TypeName##PropertyRegistrator()         \
        {                                       \
            TypeName::register_properties();    \
        }                                       \
    } g_##TypeName##PropertyRegistrator;

template<typename ...Attrs>
constexpr auto setup_attributes(Attrs&&... attrs)
{
    return std::make_tuple(std::forward<Attrs>(attrs)...);
}

#define FE_REGISTER_PROPERTY(TypeName, PropertyName, ...)                                           \
    class PropertyRegistrator_##PropertyName : public PropertyImpl<decltype(TypeName::PropertyName)>\
    {                                                                                               \
    public:                                                                                         \
        PropertyRegistrator_##PropertyName(const char* name)                                        \
            : PropertyImpl<decltype(TypeName::PropertyName)>(name) { }                              \
        virtual uint64 get_offset() const override { return offsetof(TypeName, PropertyName); }     \
        virtual uint64 get_size() const override { return sizeof(TypeName::PropertyName); }         \
        virtual bool has_attribute(const char* attrName) const override                             \
        {                                                                                           \
            constexpr static auto attributes = setup_attributes(__VA_ARGS__);                       \
            bool result = false;                                                                    \
            std::apply([&](const auto&... args)                                                     \
            {                                                                                       \
                ((                                                                                  \
                    result = strcmp(attrName, args.get_name()) == 0                                 \
                ), ...);                                                                            \
            }, attributes);                                                                         \
            return result;                                                                          \
        }                                                                                           \
        virtual const void* get_attribute(const char* attrName) const override                      \
        {                                                                                           \
            constexpr static auto attributes = setup_attributes(__VA_ARGS__);                       \
            const void* result = nullptr;                                                           \
            std::apply([&](const auto&... args)                                                     \
            {                                                                                       \
                ((                                                                                  \
                    result = strcmp(attrName, args.get_name()) == 0 ? &args : nullptr               \
                ), ...);                                                                            \
            }, attributes);                                                                         \
            return result;                                                                          \
        }                                                                                           \
    };                                                                                              \
    add_property(const_cast<TypeInfo*>(TypeName::get_static_type_info()),                           \
        allocate_property<PropertyRegistrator_##PropertyName>(#PropertyName));

}