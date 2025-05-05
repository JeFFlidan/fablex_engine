#include "opaque_material.h"
#include "asset_manager/asset_manager.h"

namespace fe::asset
{

FE_DEFINE_OBJECT(OpaqueMaterial, Material);
FE_BEGIN_PROPERTY_REGISTER(OpaqueMaterial)
{

}
FE_END_PROPERTY_REGISTER(OpaqueMaterial);

void OpaqueMaterial::serialize(Archive& archive) const
{
    Asset::serialize(archive);

    archive << m_baseColorTextureUUID;
    archive << m_normalTextureUUID;
    archive << m_roughnessTextureUUID;
    archive << m_metallicTextureUUID;
    archive << m_ambientOcclusionTextureUUID;
    archive << m_baseColor;
    archive << m_roughness;
    archive << m_metallic;
}

void OpaqueMaterial::deserialize(Archive& archive)
{
    Asset::deserialize(archive);
    
    archive >> m_baseColorTextureUUID;
    archive >> m_normalTextureUUID;
    archive >> m_roughnessTextureUUID;
    archive >> m_metallicTextureUUID;
    archive >> m_ambientOcclusionTextureUUID;
    archive >> m_baseColor;
    archive >> m_roughness;
    archive >> m_metallic;
}

void OpaqueMaterial::set_base_color_texture(Texture* baseColorTexture)
{
    FE_CHECK(baseColorTexture);
    m_baseColorTextureUUID = baseColorTexture->get_uuid();
}

void OpaqueMaterial::set_normal_texture(Texture* normalMapTexture)
{
    FE_CHECK(normalMapTexture);
    m_normalTextureUUID = normalMapTexture->get_uuid();
}

void OpaqueMaterial::set_roughness_texture(Texture* roughnessTexture)
{
    FE_CHECK(roughnessTexture);
    m_roughnessTextureUUID = roughnessTexture->get_uuid();
}

void OpaqueMaterial::set_metallic_texture(Texture* metallicTexture)
{
    FE_CHECK(metallicTexture);
    m_metallicTextureUUID = metallicTexture->get_uuid();
}

void OpaqueMaterial::set_ambient_occlusion_texture(Texture* ambientOcclusionTexture)
{
    FE_CHECK(ambientOcclusionTexture);
    m_ambientOcclusionTextureUUID = ambientOcclusionTexture->get_uuid();
}

Texture* OpaqueMaterial::base_color_texture() const
{
    if ((uint64)m_baseColorTextureUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_baseColorTextureUUID);
}

Texture* OpaqueMaterial::normal_texture() const
{
    if ((uint64)m_normalTextureUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_normalTextureUUID);
}

Texture* OpaqueMaterial::roughness_texture() const
{
    if ((uint64)m_roughnessTextureUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_roughnessTextureUUID);
}

Texture* OpaqueMaterial::metallic_texture() const
{
    if ((uint64)m_metallicTextureUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_metallicTextureUUID);
}

Texture* OpaqueMaterial::ambient_occlusion_texture() const
{
    if ((uint64)m_ambientOcclusionTextureUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_ambientOcclusionTextureUUID);
}

}