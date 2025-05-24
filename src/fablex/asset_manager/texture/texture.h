#pragma once

#include "asset_manager/asset.h"
#include "asset_manager/common.h"
#include "core/json_serialization.h"
#include "rhi/resources.h"
#include "core/events/event.h"

namespace fe::asset
{

struct TextureProxy;
class Texture;

struct TextureCreateInfo : public CreateInfo
{
    
};

struct TextureImportContext : public ImportContext
{
    
};

struct TextureImportFromMemoryContext : public ImportContext
{
    std::string name;
    const void* data = nullptr;
    uint64 dataSize = 0;
};

struct TextureImportResult
{
    Texture* texture = nullptr;
};

enum class MipmapMode
{
    BASE_MIPMAPPING,
    NO_MIPMAPS
};

// I have to implement it in the engine
enum class RuntimeCompressionMode
{
    DISABLED,
    DXT1,
    DXT5
};

class Texture : public Asset
{
    FE_DECLARE_OBJECT(Texture);
    FE_DECLARE_PROPERTY_REGISTER(Texture);

    friend TextureProxy;

public:
    // ========== Begin Object interface ==========

    virtual void serialize(Archive& archive) const override;
    virtual void deserialize(Archive& archive) override;

    // ========== End Object interface ==========

    // ========== Begin Asset interface ==========

    virtual Type get_type() const override { return Type::TEXTURE; }

    // ========== End Asset interface ==========

    void set_mipmap_mode(MipmapMode mode) 
    { 
        make_dirty();
        m_mipmapMode = mode;
    }

    void set_runtime_compressiong_mode(RuntimeCompressionMode mode) 
    {
        make_dirty(); 
        m_runtimeCompressionMode = mode; 
    }

    void set_tiling_x(rhi::AddressMode tiling)
    {
        make_dirty();
        m_tilingX = tiling;
    }

    void set_tiling_y(rhi::AddressMode tiling)
    {
        make_dirty();
        m_tilingY = tiling;
    }

    void set_tiling(rhi::AddressMode tilingX, rhi::AddressMode tilingY)
    {
        make_dirty();
        m_tilingX = tilingX;
        m_tilingY = tilingY;
    }

    void set_brightness(float brightness)
    {
        make_dirty();
        m_brightness = brightness;
    }

    void set_saturation(float saturation)
    {
        make_dirty();
        m_saturation = saturation;
    }

    rhi::Buffer* upload_buffer() const { return m_uploadBuffer; }
    uint64 size() const { return m_uploadBuffer->size; }
    uint64 width() const { return m_width; }
    uint64 height() const { return m_height; }
    uint64 depth() const { return m_depth; }
    MipmapMode mipmap_mode() const { return m_mipmapMode; }
    RuntimeCompressionMode runtime_compression_mode() const { return m_runtimeCompressionMode; }
    rhi::AddressMode tiling_x() const { return m_tilingX; }
    rhi::AddressMode tiling_y() const { return m_tilingY; }
    rhi::Format format() const { return m_format; }
    rhi::Format bc_format() const { return m_bcFormat; }
    const rhi::ComponentMapping& component_mapping() const { return m_mapping; }
    const std::vector<rhi::MipMap>& mipmaps() const { return m_mipmaps; }
    float brightness() const { return m_brightness; }
    float saturation() const { return m_saturation; }
    bool is_16bit() const { return m_is16Bit; }

protected:
    rhi::Buffer* m_uploadBuffer = nullptr;
    uint64 m_width = 0;
    uint64 m_height = 0;
    uint64 m_depth = 0;
    MipmapMode m_mipmapMode = MipmapMode::BASE_MIPMAPPING;
    RuntimeCompressionMode m_runtimeCompressionMode = RuntimeCompressionMode::DISABLED;
    rhi::AddressMode m_tilingX = rhi::AddressMode::REPEAT;
    rhi::AddressMode m_tilingY = rhi::AddressMode::REPEAT;
    rhi::Format m_format = rhi::Format::UNDEFINED;
    rhi::Format m_bcFormat = rhi::Format::UNDEFINED;
    rhi::ComponentMapping m_mapping;
    std::vector<rhi::MipMap> m_mipmaps;
    float m_brightness = 1.0f;
    float m_saturation = 1.0f;
    bool m_is16Bit = false;
};

FE_DEFINE_ASSET_POOL_SIZE(Texture, 256);

#ifdef FE_TEXTURE_PROXY

struct TextureProxy
{
    TextureProxy(Texture* texture) :
        uploadBuffer(texture->m_uploadBuffer),
        width(texture->m_width),
        height(texture->m_height),
        depth(texture->m_depth),
        format(texture->m_format),
        bcFormat(texture->m_bcFormat),
        mapping(texture->m_mapping),
        mipmaps(texture->m_mipmaps),
        is16Bit(texture->m_is16Bit)
    {

    }

    rhi::Buffer*& uploadBuffer;
    uint64& width;
    uint64& height;
    uint64& depth;
    rhi::Format& format;
    rhi::Format& bcFormat;
    rhi::ComponentMapping& mapping;
    std::vector<rhi::MipMap>& mipmaps;
    bool& is16Bit;
};

#endif // FE_TEXTURE_PROXY

}

FE_SERIALIZE_ENUM(fe::asset, fe::asset::MipmapMode,
{
    {fe::asset::MipmapMode::NO_MIPMAPS, "NO_MIPMAPS"},
    {fe::asset::MipmapMode::BASE_MIPMAPPING, "BASE_MIPMAPPING"}
})

FE_SERIALIZE_ENUM(fe::asset, fe::asset::RuntimeCompressionMode, 
{
    {fe::asset::RuntimeCompressionMode::DISABLED, "DISABLED"},
    {fe::asset::RuntimeCompressionMode::DXT1, "DXT1"},
    {fe::asset::RuntimeCompressionMode::DXT5, "DXT5"}
})
