#include "property.h"

namespace fe
{

template<>
void Property::get_value(Object* object, std::string& outValue)
{
    FE_CHECK(get_type() == PropertyType::STRING);
    std::string* stringData = (std::string*)(((char*)object) + get_offset());
    outValue = *stringData;
}

}