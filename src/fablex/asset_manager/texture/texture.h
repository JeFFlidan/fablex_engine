#pragma once

#include "asset_manager/asset.h"

namespace fe::asset
{

struct TextureCreateInfo
{
    std::string name;
};

class Texture : public Asset
{
    FE_DECLARE_OBJECT(Texture);
    FE_DECLARE_PROPERTY_REGISTER(Texture);

public:
    // ========== Begin Asset interface ==========

    virtual Type get_type() const override { return Type::TEXTURE; }

    // ========== End Asset interface ==========
};

FE_DEFINE_ASSET_POOL_SIZE(Texture, 256);

}