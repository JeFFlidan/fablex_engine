#include "material_component.h"
#include "asset_manager/asset_manager.h"

namespace fe::engine
{

void MaterialComponent::set_material(asset::Material* material)
{
    m_materialUUID = material->get_uuid();
}

void MaterialComponent::set_material_uuid(UUID materialUUID)
{
    m_materialUUID = materialUUID;
}

asset::Material* MaterialComponent::get_material() const
{
    return asset::AssetManager::get_material(m_materialUUID);
}

UUID MaterialComponent::get_material_uuid() const
{
    return m_materialUUID;
}

bool MaterialComponent::is_material_loaded() const
{
    return asset::AssetManager::is_asset_loaded(m_materialUUID);
}

}