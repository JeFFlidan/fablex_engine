#pragma once

#include "object.h"
#include "attribute.h"
#include "core/types.h"
#include "core/math.h"
#include "core/macro.h"
#include "core/logger.h"
#include "core/uuid.h"
#include <string>
#include <utility>

namespace fe
{

enum class PropertyType
{
    BOOL,
    INTEGER,
    UUID,
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    FLOAT3X4,
    FLOAT4X4,
    QUAT,
    STRING,
    ARRAY
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
FE_DEFINE_PROPERTY_TYPE_ENUM_MAPPER(UUID, UUID);
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
    Property(const char* name)  
        : m_name(name)
    {
    if (m_name.find("m_") == 0)
        m_name = m_name.substr(2);
    if (!m_name.empty())
        m_name[0] = std::toupper(m_name[0]);
    }

    virtual ~Property() = default;

    virtual PropertyType get_type() const = 0;
    virtual uint64 get_offset() const = 0;
    virtual uint64 get_size() const = 0;

    const std::string& get_name() const { return m_name; }
    
    template<typename T>
    T& get_value(Object* object)
    {
        if (get_type() == PropertyType::ARRAY)
            FE_CHECK_MSG(0, "Can't use get_value method for ArrayProperty.");
        return *static_cast<T*>(get_property_ptr(object));
    }

    template<typename T>
    const T& get_value(Object* object) const
    {
        if (get_type() == PropertyType::ARRAY)
            FE_CHECK_MSG(0, "Can't use get_value method for ArrayProperty.");
        return *static_cast<T*>(get_property_ptr(object)); 
    }

    template<typename T>
    void set_value(Object* object, const T& value)
    {
        if (get_type() == PropertyType::ARRAY)
            FE_CHECK_MSG(0, "Can't use set_value method for ArrayProperty.");
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
    std::string m_name;

    virtual bool has_attribute(const char* attrName) const { return false; }
    virtual const void* get_attribute(const char* attrName) const { return nullptr; }

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
    PropertyImpl(const char* name) : Property(name) { }

    virtual PropertyType get_type() const override { return PropertyTypeEnumMapper<T>::value; }
    virtual uint64 get_offset() const override { return 0; };
    virtual uint64 get_size() const override { return 0; };
};

class ArrayProperty : public Property
{
public:
    ArrayProperty(const char* name) : Property(name) { }

    virtual PropertyType get_type() const override { return PropertyType::ARRAY; }
    virtual uint64 get_offset() const override { return 0; }
    virtual uint64 get_size() const override { return 0; }
    virtual uint64 get_element_count(Object* object) const { return 0; }
    virtual void* get_data(Object* object) { return nullptr; }
    virtual const void* get_data(Object* object) const { return nullptr; } 

    template<typename T>
    T& get_value(Object* object, uint64 index)
    {
        return *static_cast<T*>(get_array_value_internal(object, index));
    }

    // If property type is not ARRAY, returns nullptr
    template<typename T>
    const T& get_value(Object* object, uint64 index) const
    {
        return *static_cast<T*>(get_array_value_internal(object, index));
    }

    // Pushes back to array
    template<typename T>
    void add_value(Object* object, const T& value)
    {
        add_array_value_internal(object, &value);
    }

    template<typename T>
    void set_value(Object* object, const T& value, uint64 index)
    {
        set_array_value_internal(object, &value, index);
    }

protected:
    virtual void* get_array_value_internal(Object* object, uint64 index) const = 0;
    virtual void add_array_value_internal(Object* object, const void* value) = 0;
    virtual void set_array_value_internal(Object* object, const void* value, uint64 index) = 0;

    template<typename ValueType>
    std::vector<ValueType>& get_array(Object* object) const
    {
        return *reinterpret_cast<std::vector<ValueType>*>(get_property_ptr(object));
    }
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

#define FE_DEFINE_ATTR_METHODS(...)                                                             \
    virtual bool has_attribute(const char* attrName) const override                             \
    {                                                                                           \
        constexpr static auto attributes = setup_attributes(__VA_ARGS__);                       \
        return std::apply([&](const auto&... args)                                              \
        {                                                                                       \
            return ( (strcmp(attrName, args.get_name()) == 0) || ... );                         \
        }, attributes);                                                                         \
    }                                                                                           \
    virtual const void* get_attribute(const char* attrName) const override                      \
    {                                                                                           \
        constexpr static auto attributes = setup_attributes(__VA_ARGS__);                       \
        return std::apply(                                                                      \
            [&](auto const&... args)                                                            \
            {                                                                                   \
                const void* res = nullptr;                                                      \
                (( strcmp(attrName, args.get_name()) == 0                                       \
                        && (res = &args, true) )                                                \
                 || ...);                                                                       \
                return res;                                                                     \
            },                                                                                  \
            attributes                                                                          \
        );                                                                                      \
    }

#define FE_REGISTER_PROPERTY(TypeName, PropertyName, ...)                                           \
    class PropertyRegistrator_##PropertyName : public PropertyImpl<decltype(TypeName::PropertyName)>\
    {                                                                                               \
    public:                                                                                         \
        PropertyRegistrator_##PropertyName(const char* name)                                        \
            : PropertyImpl<decltype(TypeName::PropertyName)>(name) { }                              \
        virtual uint64 get_offset() const override { return offsetof(TypeName, PropertyName); }     \
        virtual uint64 get_size() const override { return sizeof(TypeName::PropertyName); }         \
        FE_DEFINE_ATTR_METHODS(__VA_ARGS__)                                                         \
    };                                                                                              \
    add_property(const_cast<TypeInfo*>(TypeName::get_static_type_info()),                           \
        allocate_property<PropertyRegistrator_##PropertyName>(#PropertyName));

#define FE_REGISTER_ARRAY_PROPERTY(TypeName, PropertyName, ...)                                     \
    class PropertyRegistrator_##PropertyName : public ArrayProperty                                 \
    {                                                                                               \
    public:                                                                                         \
        using ValueType = decltype(TypeName::PropertyName)::value_type;                             \
        PropertyRegistrator_##PropertyName(const char* name) : ArrayProperty(name) { }              \
        virtual uint64 get_offset() const override { return offsetof(TypeName, PropertyName); }     \
        virtual uint64 get_size() const override { return sizeof(ValueType); }                      \
        virtual void* get_data(Object* object) override { return get_array(object).get_data(); }    \
        virtual const void* get_data(Object* object) const override { return get_array(object).get_data(); }    \
        virtual uint64 get_element_count(Object* object) const override                             \
        {                                                                                           \
            return get_array<ValueType>(object).size();                                             \
        }                                                                                           \
        FE_DEFINE_ATTR_METHODS(__VA_ARGS__)                                                         \
        virtual void* get_array_value_internal(Object* object, uint64 index) const override         \
        {                                                                                           \
            return &get_array<ValueType>(object).at(index);                                         \
        }                                                                                           \
        virtual void add_array_value_internal(Object* object, const void* value) override           \
        {                                                                                           \
            get_array<ValueType>(object).push_back(*static_cast<const ValueType*>(value));          \
        }                                                                                           \
        virtual void set_array_value_internal(Object* object, const void* value, uint64 index) override\
        {                                                                                           \
            std::vector<ValueType>& vec = get_array<ValueType>(object);                             \
            if (index >= vec.size())                                                                \
            {                                                                                       \
                FE_LOG(LogDefault, ERROR, "ArrayProperty::set_array_value(): Index is {}, array size is {}", index, vec.size());\
                return;                                                                             \
            }                                                                                       \
            vec[index] = *static_cast<const ValueType*>(value);                                     \
        }                                                                                           \
    };                                                                                              \
    add_property(const_cast<TypeInfo*>(TypeName::get_static_type_info()),                           \
        allocate_property<PropertyRegistrator_##PropertyName>(#PropertyName));

}