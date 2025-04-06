#pragma once

#include "asset_manager/asset.h"

namespace fe::asset
{

struct MaterialCreateInfo
{
    std::string name;
};

class Material : public Asset
{
    FE_DECLARE_OBJECT(Material)
    FE_DECLARE_PROPERTY_REGISTER(Material)

public:
    // ========== Begin Asset interface ==========

    virtual Type get_type() const override { return Type::MATERIAL; }

    // ========== End Asset interface ==========

protected:
};

FE_DEFINE_ASSET_POOL_SIZE(Material, 256);

}