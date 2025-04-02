#include "property.h"

namespace fe
{

template<>
std::string& Property::get_value(Object* object)
{
    FE_CHECK(get_type() == PropertyType::STRING);
    std::string* stringData = (std::string*)(((char*)object) + get_offset());
    return *stringData;
}

}