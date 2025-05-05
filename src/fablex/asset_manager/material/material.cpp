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
}

void Material::deserialize(Archive& archive)
{
    Asset::deserialize(archive);
}

}