#pragma once

#include "asset_manager/common.h"
#include "asset_manager/asset.h"

namespace fe::asset
{

class Texture;

enum class MaterialType
{
    UNDEFINED,
    OPAQUE,
};

struct MaterialCreateInfo : public CreateInfo
{
    MaterialType type;
};

// This material implementation is temp
class Material : public Asset
{
    FE_DECLARE_OBJECT(Material)
    FE_DECLARE_PROPERTY_REGISTER(Material)

    friend AssetManager;

public:
    // ========== Begin Object interface ==========

    virtual void serialize(Archive& archive) const override;
    virtual void deserialize(Archive& archive) override;

    // ========== End Object interface ==========

    // ========== Begin Asset interface ==========

    virtual Type get_type() const override { return Type::MATERIAL; }
    MaterialType get_material_type() const { return m_materialType; } 

    // ========== End Asset interface ==========

protected:
    MaterialType m_materialType;
};

}