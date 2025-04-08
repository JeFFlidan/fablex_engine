#pragma once

#include "asset_manager/common.h"
#include "asset_manager/asset.h"

namespace fe::asset
{

class Texture;

struct MaterialCreateInfo : public CreateInfo
{
    
};

// This material implementation is temp
class Material : public Asset
{
    FE_DECLARE_OBJECT(Material)
    FE_DECLARE_PROPERTY_REGISTER(Material)

public:
    // ========== Begin Object interface ==========

    virtual void serialize(Archive& archive) const override;
    virtual void deserialize(Archive& archive) override;

    // ========== End Object interface ==========

    // ========== Begin Asset interface ==========

    virtual Type get_type() const override { return Type::MATERIAL; }

    // ========== End Asset interface ==========

    void set_base_color(Texture* baseColorTexture);
    void set_normal(Texture* normalMapTexture);
    void set_roughness(Texture* roughnessTexture);
    void set_metallic(Texture* metallicTexture);
    void set_ambient_occlusion(Texture* ambientOcclusionTexture);

    Texture* base_color() const;
    Texture* normal() const;
    Texture* roughness() const;
    Texture* metallic() const;
    Texture* ambient_occlusion() const;

protected:
    UUID m_baseColorUUID = UUID::INVALID;
    UUID m_normalMapUUID = UUID::INVALID;
    UUID m_roughnessUUID = UUID::INVALID;
    UUID m_metallicUUID = UUID::INVALID;
    UUID m_ambientOcclusionUUID = UUID::INVALID;
};

FE_DEFINE_ASSET_POOL_SIZE(Material, 256);

}