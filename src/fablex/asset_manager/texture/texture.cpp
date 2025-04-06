#include "texture.h"
#include "core/file_system/archive.h"
#include "rhi/json_serialization.h"

namespace fe::asset
{

FE_DEFINE_OBJECT(Texture, Asset);

FE_BEGIN_PROPERTY_REGISTER(Texture)
{

}
FE_END_PROPERTY_REGISTER(Texture);

void Texture::serialize(Archive& archive) const
{
    Asset::serialize(archive);

    archive << m_data;
    archive << m_width;
    archive << m_height;
    archive << m_depth;
    archive << to_string(m_mipmapMode);
    archive << to_string(m_runtimeCompressionMode);
    archive << to_string(m_tilingX);
    archive << to_string(m_tilingY);
    archive << to_string(m_format);
    archive << to_string(m_bcFormat);
    archive << to_string(m_mapping.r);
    archive << to_string(m_mapping.g);
    archive << to_string(m_mapping.b);
    archive << to_string(m_mapping.a);
    archive << m_brightness;
    archive << m_saturation;
    archive << m_is16Bit;
}

void Texture::deserialize(Archive& archive)
{
    Asset::deserialize(archive);

    archive >> m_data;
    archive >> m_width;
    archive >> m_height;
    archive >> m_depth;

    std::string result;
    
    archive >> result;
    to_enum(result, m_mipmapMode);
    
    archive >> result;
    to_enum(result, m_runtimeCompressionMode);

    archive >> result;
    to_enum(result, m_tilingX);

    archive >> result;
    to_enum(result, m_tilingY);

    archive >> result;
    to_enum(result, m_format);

    archive >> result;
    to_enum(result, m_bcFormat);

    archive >> result;
    to_enum(result, m_mapping.r);

    archive >> result;
    to_enum(result, m_mapping.g);

    archive >> result;
    to_enum(result, m_mapping.b);

    archive >> result;
    to_enum(result, m_mapping.a);

    archive >> m_brightness;
    archive >> m_saturation;
    archive >> m_is16Bit;
}

}