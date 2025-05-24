#include "opaque_material_settings.h"
#include "asset_manager.h"
#include "renderer/utils.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::asset
{

FE_DEFINE_OBJECT(OpaqueMaterialSettings, Object);
FE_BEGIN_PROPERTY_REGISTER(OpaqueMaterialSettings)
{

}
FE_END_PROPERTY_REGISTER(OpaqueMaterialSettings)

void OpaqueMaterialSettings::fill_shader_material(ShaderMaterial& outShaderMaterial)
{
    outShaderMaterial.set_base_color(m_baseColor);
    outShaderMaterial.set_metallic(m_metallic);
    outShaderMaterial.set_roughness(m_roughness);
    outShaderMaterial.set_sampler_index(renderer::Utils::get_sampler_linear_repeat());

    set_texture(outShaderMaterial, TEXTURE_SLOT_BASE_COLOR, base_color_texture());
    set_texture(outShaderMaterial, TEXTURE_SLOT_NORMAL, normal_texture());
    set_texture(outShaderMaterial, TEXTURE_SLOT_ROUGHNESS, roughness_texture());
    set_texture(outShaderMaterial, TEXTURE_SLOT_METALLIC, metallic_texture());
    set_texture(outShaderMaterial, TEXTURE_SLOT_AO, ambient_occlusion_texture());
    set_texture(outShaderMaterial, TEXTURE_SLOT_ARM, arm_texture());
}

void OpaqueMaterialSettings::serialize(Archive& archive) const
{
    MaterialSettings::serialize(archive);

    archive << m_baseColorTextureUUID;
    archive << m_normalTextureUUID;
    archive << m_roughnessTextureUUID;
    archive << m_metallicTextureUUID;
    archive << m_ambientOcclusionTextureUUID;
    archive << m_armTextureUUID;
    archive << m_baseColor;
    archive << m_roughness;
    archive << m_metallic;
}

void OpaqueMaterialSettings::deserialize(Archive& archive)
{
    MaterialSettings::deserialize(archive);
    
    archive >> m_baseColorTextureUUID;
    archive >> m_normalTextureUUID;
    archive >> m_roughnessTextureUUID;
    archive >> m_metallicTextureUUID;
    archive >> m_ambientOcclusionTextureUUID;
    archive >> m_armTextureUUID;
    archive >> m_baseColor;
    archive >> m_roughness;
    archive >> m_metallic;
}

void OpaqueMaterialSettings::init(const OpaqueMaterialCreateInfo& info)
{
    if (info.baseColorTexture)
        set_base_color_texture(info.baseColorTexture);
    if (info.normalTexture)
        set_normal_texture(info.normalTexture);
    if (info.roughnessTexture)
        set_roughness_texture(info.roughnessTexture);
    if (info.metallicTexture)
        set_metallic_texture(info.metallicTexture);
    if (info.ambientOcclusionTexture)
        set_ambient_occlusion_texture(info.ambientOcclusionTexture);
    if (info.armTexture)
        set_arm_texture(info.armTexture);

    set_base_color(info.baseColor);
    set_roughness(info.roughness);
    set_metallic(info.metallic);
}

void OpaqueMaterialSettings::set_base_color_texture(Texture* baseColorTexture)
{
    FE_CHECK(baseColorTexture);
    m_baseColorTextureUUID = baseColorTexture->get_uuid();
}

void OpaqueMaterialSettings::set_normal_texture(Texture* normalMapTexture)
{
    FE_CHECK(normalMapTexture);
    m_normalTextureUUID = normalMapTexture->get_uuid();
}

void OpaqueMaterialSettings::set_roughness_texture(Texture* roughnessTexture)
{
    FE_CHECK(roughnessTexture);
    m_roughnessTextureUUID = roughnessTexture->get_uuid();
}

void OpaqueMaterialSettings::set_metallic_texture(Texture* metallicTexture)
{
    FE_CHECK(metallicTexture);
    m_metallicTextureUUID = metallicTexture->get_uuid();
}

void OpaqueMaterialSettings::set_ambient_occlusion_texture(Texture* ambientOcclusionTexture)
{
    FE_CHECK(ambientOcclusionTexture);
    m_ambientOcclusionTextureUUID = ambientOcclusionTexture->get_uuid();
}

void OpaqueMaterialSettings::set_arm_texture(Texture* armTexture)
{
    FE_CHECK(armTexture);
    m_armTextureUUID = armTexture->get_uuid();
}

Texture* OpaqueMaterialSettings::base_color_texture() const
{
    if ((uint64)m_baseColorTextureUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_baseColorTextureUUID);
}

Texture* OpaqueMaterialSettings::normal_texture() const
{
    if ((uint64)m_normalTextureUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_normalTextureUUID);
}

Texture* OpaqueMaterialSettings::roughness_texture() const
{
    if ((uint64)m_roughnessTextureUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_roughnessTextureUUID);
}

Texture* OpaqueMaterialSettings::metallic_texture() const
{
    if ((uint64)m_metallicTextureUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_metallicTextureUUID);
}

Texture* OpaqueMaterialSettings::ambient_occlusion_texture() const
{
    if ((uint64)m_ambientOcclusionTextureUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_ambientOcclusionTextureUUID);
}

Texture* OpaqueMaterialSettings::arm_texture() const
{
        if ((uint64)m_armTextureUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_armTextureUUID);
}

}