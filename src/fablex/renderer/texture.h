#pragma once

#include "core/types.h"
#include "rhi/resources.h"

namespace fe::renderer
{

class Texture
{
public:
    Texture() = default;
    Texture(rhi::TextureHandle handle);
    ~Texture();

    rhi::TextureHandle get_handle() const { return m_handle; }

    rhi::TextureViewHandle get_dsv() const;
    rhi::TextureViewHandle get_srv() const;
    rhi::TextureViewHandle get_rtv(uint32 mipLevel = 0) const;
    rhi::TextureViewHandle get_uav(uint32 mipLevel = 0) const;

    uint32 get_dsv_descriptor() const;
    uint32 get_srv_descriptor() const;
    uint32 get_rtv_descriptor(uint32 mipLevel = 0) const;
    uint32 get_uav_descriptor(uint32 mipLevel = 0) const;

private:
    rhi::TextureHandle m_handle = nullptr;
    mutable rhi::TextureViewHandle m_dsTextureView = nullptr;
    mutable rhi::TextureViewHandle m_srTextureView = nullptr;
    mutable std::vector<rhi::TextureViewHandle> m_rtTextureViews;
    mutable std::vector<rhi::TextureViewHandle> m_uaTextureViews;

    void reserve_texture_view_arrays();
};

}