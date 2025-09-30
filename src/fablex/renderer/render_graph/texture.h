#pragma once

#include "core/types.h"
#include "core/name.h"
#include "rhi/resources.h"

namespace fe::renderer
{

class Texture
{
public:
    Texture() = default;
    Texture(rhi::TextureHandle handle, Name textureName);
    ~Texture();

    rhi::TextureHandle handle() const { return m_handle; }

    rhi::TextureViewHandle dsv() const;
    rhi::TextureViewHandle srv() const;
    rhi::TextureViewHandle rtv(uint32 mipLevel = 0) const;
    rhi::TextureViewHandle uav(uint32 mipLevel = 0) const;

    uint32 dsv_descriptor() const;
    uint32 srv_descriptor() const;
    uint32 rtv_descriptor(uint32 mipLevel = 0) const;
    uint32 uav_descriptor(uint32 mipLevel = 0) const;

private:
    Name m_name;
    rhi::TextureHandle m_handle = nullptr;
    mutable rhi::TextureViewHandle m_dsTextureView = nullptr;
    mutable rhi::TextureViewHandle m_srTextureView = nullptr;
    mutable std::vector<rhi::TextureViewHandle> m_rtTextureViews;
    mutable std::vector<rhi::TextureViewHandle> m_uaTextureViews;

    void reserve_texture_view_arrays();
    std::string get_view_name(uint32 mipLevel) const;
};

}