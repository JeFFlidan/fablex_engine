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

    archive << m_materialSettings->get_type_info()->get_name();
    m_materialSettings->serialize(archive);
}

void Material::deserialize(Archive& archive)
{
    Asset::deserialize(archive);

    std::string typeName;
    archive >> typeName;
    m_materialSettings = static_cast<MaterialSettings*>(create_object(typeName));

    m_materialSettings->deserialize(archive);
}

}