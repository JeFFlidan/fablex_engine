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

}