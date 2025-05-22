#include "material_component.h"
#include "asset_manager/asset_manager.h"

namespace fe::engine
{

FE_DEFINE_OBJECT(MaterialComponent, Component);
FE_BEGIN_PROPERTY_REGISTER(MaterialComponent)
{

}
FE_END_PROPERTY_REGISTER(MaterialComponent)

void MaterialComponent::add_material(asset::Material* material)
{
    m_materialUUIDs.reserve(100);
    m_materialUUIDs.push_back(material->get_uuid());
}

void MaterialComponent::add_material(UUID materialUUID)
{
    m_materialUUIDs.push_back(materialUUID);
}

}