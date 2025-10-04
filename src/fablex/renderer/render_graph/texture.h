#pragma once

#include "core/types.h"
#include "core/name.h"
#include "rhi/resources/texture.h"
#include "rhi/resources/texture_view.h"
#include <vector>

namespace fe::renderer
{

class Texture
{
public:
    Texture() = default;
    Texture(rhi::Texture* handle, Name textureName);
    ~Texture();

    rhi::Texture* handle() const { return m_handle; }

    rhi::TextureView* dsv() const;
    rhi::TextureView* srv() const;
    rhi::TextureView* rtv(uint32 mipLevel = 0) const;
    rhi::TextureView* uav(uint32 mipLevel = 0) const;

    uint32 dsv_descriptor() const;
    uint32 srv_descriptor() const;
    uint32 rtv_descriptor(uint32 mipLevel = 0) const;
    uint32 uav_descriptor(uint32 mipLevel = 0) const;

private:
    Name m_name;
    rhi::Texture* m_handle = nullptr;
    mutable rhi::TextureView* m_dsTextureView = nullptr;
    mutable rhi::TextureView* m_srTextureView = nullptr;
    mutable std::vector<rhi::TextureView*> m_rtTextureViews;
    mutable std::vector<rhi::TextureView*> m_uaTextureViews;

    void reserve_texture_view_arrays();
    std::string get_view_name(uint32 mipLevel) const;
};

}