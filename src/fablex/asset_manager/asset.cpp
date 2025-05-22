#include "asset.h"
#include "core/file_system/archive.h"

namespace fe::asset
{

FE_DEFINE_OBJECT(Asset, Object);

FE_BEGIN_PROPERTY_REGISTER(Asset)
{

}
FE_END_PROPERTY_REGISTER(Asset);

void Asset::serialize(Archive& archive) const
{
    Object::serialize(archive);

    archive.set_uuid(m_uuid);
    archive.set_object_type<Type>(get_type());

    archive << m_name;
    archive << m_assetPath;
    archive << m_originalFilePath;
    archive << std::to_underlying<AssetFlag>(m_assetFlag);
}

void Asset::deserialize(Archive& archive)
{
    Object::deserialize(archive);

    m_uuid = archive.get_uuid();

    archive >> m_name;
    archive >> m_assetPath;
    archive >> m_originalFilePath;
    
    uint64 flags;
    archive >> flags;
    m_assetFlag = static_cast<AssetFlag>(flags);
}

uint32 get_asset_type_count()
{
    return std::to_underlying(Type::COUNT);
}

}