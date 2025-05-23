#pragma once

namespace fe
{

struct Attribute
{

};

#define FE_DEFINE_ATTRIBUTE(Attr)               \
    constexpr static const char* name = #Attr;  \
    constexpr const char* get_name() const { return name; } 

struct Binary : public Attribute
{
    FE_DEFINE_ATTRIBUTE(Binary);
};

struct EditAnywhere : public Attribute
{
    FE_DEFINE_ATTRIBUTE(EditAnywhere);
};

struct Transient : public Attribute
{
    FE_DEFINE_ATTRIBUTE(Transient)
};

struct Color : public Attribute
{
    FE_DEFINE_ATTRIBUTE(Color);
};

struct ClampMin : public Attribute
{
    FE_DEFINE_ATTRIBUTE(ClampMin);
    constexpr ClampMin(float inMin) : min(inMin) { }

    float min = 0.0f;
};

struct ClampMax : public Attribute
{
    FE_DEFINE_ATTRIBUTE(ClampMax);
    constexpr ClampMax(float inMax) : max(inMax) { }

    float max = 0.0f;
};

struct SliderSpeed : public Attribute
{
    FE_DEFINE_ATTRIBUTE(SliderSpeed);
    constexpr SliderSpeed(float inSpeed) : speed(inSpeed) { }

    float speed = 1.0f;
};

}