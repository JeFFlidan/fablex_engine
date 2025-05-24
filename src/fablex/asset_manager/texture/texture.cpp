#include "texture.h"
#include "core/file_system/archive.h"
#include "rhi/json_serialization.h"
#include "rhi/rhi.h"

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
    FE_CHECK(m_uploadBuffer);
    archive << m_uploadBuffer->size;
    uint8* typedData = static_cast<uint8*>(m_uploadBuffer->mappedData);

    for (uint32 i = 0; i != m_uploadBuffer->size; ++i)
        archive << typedData[i];

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

    archive << m_mipmaps.size();
    for (auto& mipmap : m_mipmaps)
    {
        archive << mipmap.layer;
        archive << mipmap.offset;
    }

    archive << m_brightness;
    archive << m_saturation;
    archive << m_is16Bit;
}

void Texture::deserialize(Archive& archive)
{
    Asset::deserialize(archive);

    rhi::BufferInfo bufferInfo;
    archive >> bufferInfo.size;
    bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC;
    bufferInfo.memoryUsage = rhi::MemoryUsage::CPU;
    rhi::create_buffer(&m_uploadBuffer, &bufferInfo);
    rhi::set_name(m_uploadBuffer, get_name() + "UploadBuffer");

    uint8* typedData = static_cast<uint8*>(m_uploadBuffer->mappedData);

    for (uint32 i = 0; i != m_uploadBuffer->size; ++i)
        archive >> typedData[i];

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

    uint64 mipmapCount = 0;
    archive >> mipmapCount;
    m_mipmaps.reserve(mipmapCount);
    for (uint32 i = 0; i != mipmapCount; ++i)
    {
        rhi::MipMap& mipmap = m_mipmaps.emplace_back();
        archive >> mipmap.layer;
        archive >> mipmap.offset;
    }

    archive >> m_brightness;
    archive >> m_saturation;
    archive >> m_is16Bit;
}

}