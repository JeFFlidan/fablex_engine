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

    archive << m_uuid;
    archive << m_name;
    archive << m_assetPath;
    archive << m_originalFilePath;
}

void Asset::deserialize(Archive& archive)
{
    Object::deserialize(archive);

    archive >> m_uuid;
    archive >> m_name;
    archive >> m_assetPath;
    archive >> m_originalFilePath;
}

}