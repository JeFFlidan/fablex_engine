#pragma once

#include "core/types.h"
#include "core/flags_operations.h"

namespace fe::asset
{

enum class Type : uint32
{
    MODEL,
    TEXTURE,
    MATERIAL,

    COUNT
};

enum class AssetFlag : uint64
{
    UNDEFINED = 0,
    USE_AS_DEFAULT = 1 << 0,
};

}

ENABLE_BIT_MASK(fe::asset::AssetFlag);
