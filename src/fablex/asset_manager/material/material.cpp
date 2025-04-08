#include "material.h"
#include "asset_manager/asset_manager.h"

namespace fe::asset
{

FE_DEFINE_OBJECT(Material, Asset);

FE_BEGIN_PROPERTY_REGISTER(Material)
{
    
}
FE_END_PROPERTY_REGISTER(Material);

void Material::serialize(Archive& archive) const
{
    Asset::serialize(archive);

    archive << m_baseColorUUID;
    archive << m_normalMapUUID;
    archive << m_roughnessUUID;
    archive << m_metallicUUID;
    archive << m_ambientOcclusionUUID;
}

void Material::deserialize(Archive& archive)
{
    Asset::deserialize(archive);
    
    archive >> m_baseColorUUID;
    archive >> m_normalMapUUID;
    archive >> m_roughnessUUID;
    archive >> m_metallicUUID;
    archive >> m_ambientOcclusionUUID;
}

void Material::set_base_color(Texture* baseColorTexture)
{
    FE_CHECK(baseColorTexture);
    m_baseColorUUID = baseColorTexture->get_uuid();
}

void Material::set_normal(Texture* normalMapTexture)
{
    FE_CHECK(normalMapTexture);
    m_normalMapUUID = normalMapTexture->get_uuid();
}

void Material::set_roughness(Texture* roughnessTexture)
{
    FE_CHECK(roughnessTexture);
    m_roughnessUUID = roughnessTexture->get_uuid();
}

void Material::set_metallic(Texture* metallicTexture)
{
    FE_CHECK(metallicTexture);
    m_metallicUUID = metallicTexture->get_uuid();
}

void Material::set_ambient_occlusion(Texture* ambientOcclusionTexture)
{
    FE_CHECK(ambientOcclusionTexture);
    m_ambientOcclusionUUID = ambientOcclusionTexture->get_uuid();
}

Texture* Material::base_color() const
{
    if ((uint64)m_baseColorUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_baseColorUUID);
}

Texture* Material::normal() const
{
    if ((uint64)m_normalMapUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_normalMapUUID);
}

Texture* Material::roughness() const
{
    if ((uint64)m_roughnessUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_roughnessUUID);
}

Texture* Material::metallic() const
{
    if ((uint64)m_metallicUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_metallicUUID);
}

Texture* Material::ambient_occlusion() const
{
    if ((uint64)m_ambientOcclusionUUID == UUID::INVALID)
        return nullptr;
    return AssetManager::get_texture(m_ambientOcclusionUUID);
}

}