#include "material_component.h"
#include "asset_manager/asset_manager.h"

namespace fe::engine
{

FE_DEFINE_OBJECT(MaterialComponent, Component);
FE_BEGIN_PROPERTY_REGISTER(MaterialComponent)
{
    FE_REGISTER_ARRAY_PROPERTY(MaterialComponent, m_materialUUIDs, EditAnywhere(), Type("material"));
}
FE_END_PROPERTY_REGISTER(MaterialComponent)

void MaterialComponent::init(asset::Model* model)
{
    m_materialUUIDs.resize(model->material_slots().size());

    for (uint32 i = 0; i != model->material_slots().size(); ++i)
    {
        const asset::MaterialSlot& matSlot = model->material_slots()[i];
        m_materialUUIDs[i] = matSlot.materialUUID;
    }
}

void MaterialComponent::add_material(asset::Material* material)
{
    m_materialUUIDs.reserve(100);
    m_materialUUIDs.push_back(material->get_uuid());
}

void MaterialComponent::add_material(UUID materialUUID)
{
    m_materialUUIDs.push_back(materialUUID);
}

bool MaterialComponent::set_material(asset::Material* material, uint32 index)
{
    if (index >= m_materialUUIDs.size())
        return false;

    m_materialUUIDs[index] = material->get_uuid();
    return true;
}

bool MaterialComponent::set_material(UUID materialUUID, uint32 index)
{
    if (index >= m_materialUUIDs.size())
        return false;

    m_materialUUIDs[index] = materialUUID;
    return true;
}

}