#pragma once

#include "material_settings.h"

namespace fe::asset
{

class Texture;
struct OpaqueMaterialCreateInfo;

class OpaqueMaterialSettings : public MaterialSettings
{
    FE_DECLARE_OBJECT(OpaqueMaterialSettings);
    FE_DECLARE_PROPERTY_REGISTER(OpaqueMaterialSettings);

public:
    virtual void fill_shader_material(ShaderMaterial& outShaderMaterial) override;
    virtual MaterialType material_type() const override { return MaterialType::OPAQUE; }

    // ========== Begin Object interface ==========

    virtual void serialize(Archive& archive) const override;
    virtual void deserialize(Archive& archive) override;

    // ========== End Object interface ==========

    void init(const OpaqueMaterialCreateInfo& info);

    void set_base_color_texture(Texture* baseColorTexture);
    void set_normal_texture(Texture* normalMapTexture);
    void set_roughness_texture(Texture* roughnessTexture);
    void set_metallic_texture(Texture* metallicTexture);
    void set_ambient_occlusion_texture(Texture* ambientOcclusionTexture);
    void set_arm_texture(Texture* armTexture);

    void set_base_color(const Float4& baseColor) { m_baseColor = baseColor; }
    void set_roughness(float roughness) { m_roughness = roughness; }
    void set_metallic(float metallic) { m_metallic = metallic; }

    Texture* base_color_texture() const;
    Texture* normal_texture() const;
    Texture* roughness_texture() const;
    Texture* metallic_texture() const;
    Texture* ambient_occlusion_texture() const;
    Texture* arm_texture() const;

    bool has_base_color_texture() const { return m_baseColorTextureUUID != UUID::INVALID; }
    bool has_normal_texture() const { return m_normalTextureUUID != UUID::INVALID; }
    bool has_roughness_texture() const { return m_roughnessTextureUUID != UUID::INVALID; }
    bool has_metallic_texture() const { return m_metallicTextureUUID != UUID::INVALID; }
    bool has_ambient_occlusion_texture() const { return m_ambientOcclusionTextureUUID != UUID::INVALID; }
    bool has_arm_texture() const { return m_armTextureUUID != UUID::INVALID; }

    UUID base_color_texture_uuid() const { return m_baseColorTextureUUID; }
    UUID normal_texture_uuid() const { return m_normalTextureUUID; }
    UUID roughness_texture_uuid() const { return m_roughnessTextureUUID; }
    UUID metallic_texture_uuid() const { return m_metallicTextureUUID; }
    UUID ambient_occlusion_texture_uuid() const { return m_ambientOcclusionTextureUUID; }
    UUID arm_texture_uuid() const { return m_armTextureUUID; }
    
    Float4 base_color() const { return m_baseColor; }
    float roughness() const { return m_roughness; }
    float metallic() const { return m_metallic; }

private:
    UUID m_baseColorTextureUUID = UUID::INVALID;
    UUID m_normalTextureUUID = UUID::INVALID;
    UUID m_roughnessTextureUUID = UUID::INVALID;
    UUID m_metallicTextureUUID = UUID::INVALID;
    UUID m_ambientOcclusionTextureUUID = UUID::INVALID;
    UUID m_armTextureUUID = UUID::INVALID;

    Float4 m_baseColor = Float4(1.0f, 1.0f, 1.0f, 1.0f);
    float m_roughness = 0.0f;
    float m_metallic = 0.0f;
};

struct OpaqueMaterialCreateInfo : MaterialCreateInfo
{
    OpaqueMaterialCreateInfo() 
    { 
        type = MaterialType::OPAQUE;
        initHandler = [&]()
        {
            auto settings = create_object<OpaqueMaterialSettings>();
            settings->init(*this);
            return std::move(settings);
        };
    }

    Texture* baseColorTexture = nullptr;
    Texture* normalTexture = nullptr;
    Texture* roughnessTexture = nullptr;
    Texture* metallicTexture = nullptr;
    Texture* ambientOcclusionTexture = nullptr;
    Texture* armTexture = nullptr;

    Float4 baseColor = Float4(0.8f, 0.8f, 0.8f, 1.0f);
    float roughness = 0.3f;
    float metallic = 0.0f;
};

}