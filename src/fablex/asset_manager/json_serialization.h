#pragma once

#include "enums.h"
#include "core/json_serialization.h"

FE_SERIALIZE_ENUM(fe::asset, fe::asset::Type, 
{
    {fe::asset::Type::MODEL, "MODEL"},
    {fe::asset::Type::TEXTURE, "TEXTURE"},
    {fe::asset::Type::MATERIAL, "MATERIAL"}
});
