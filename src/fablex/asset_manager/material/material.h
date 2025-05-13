#pragma once

#include "asset_manager/common.h"
#include "asset_manager/asset.h"
#include "material_settings.h"

namespace fe::asset
{

class Texture;

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
    
    // ========== End Asset interface ==========

    MaterialType material_type() const 
    { 
        return m_materialSettings->material_type(); 
    }
    
    void fill_shader_material(ShaderMaterial& shaderMaterial) 
    {
        FE_CHECK(m_materialSettings);
        m_materialSettings->fill_shader_material(shaderMaterial); 
    }

    MaterialSettings& material_settings() { return *m_materialSettings; }
    const MaterialSettings& material_settings() const { return *m_materialSettings; }

    template<typename T>
    T* material_settings() const
    {
        return static_cast<T*>(m_materialSettings.get());
    }
    
protected:
    MaterialSettingsHandle m_materialSettings = nullptr;
};

FE_DEFINE_ASSET_POOL_SIZE(Material, 256);

}