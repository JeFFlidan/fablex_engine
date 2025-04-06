#pragma once

#include "asset_manager/asset.h"
#include "core/json_serialization.h"
#include "rhi/enums.h"

namespace fe::asset
{

struct TextureProxy;

struct TextureCreateInfo
{
    std::string name;
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

    const std::vector<uint8>& data() const { return m_data; }
    uint64 size() const { return m_data.size(); }
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
    float brightness() const { return m_brightness; }
    float saturation() const { return m_saturation; }
    bool is_16bit() const { return m_is16Bit; }

protected:
    std::vector<uint8> m_data;
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
    float m_brightness = 1.0f;
    float m_saturation = 1.0f;
    bool m_is16Bit = false;
};

FE_DEFINE_ASSET_POOL_SIZE(Texture, 256);

struct TextureProxy
{
    TextureProxy(Texture* texture) :
        data(texture->m_data),
        width(texture->m_width),
        height(texture->m_height),
        depth(texture->m_depth),
        format(texture->m_format),
        bcFormat(texture->m_bcFormat),
        mapping(texture->m_mapping),
        is16Bit(texture->m_is16Bit)
    {

    }

    std::vector<uint8>& data;
    uint64& width;
    uint64& height;
    uint64& depth;
    rhi::Format& format;
    rhi::Format& bcFormat;
    rhi::ComponentMapping& mapping;
    bool& is16Bit;
};

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
