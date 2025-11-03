#pragma once

#include "core/types.h"
#include "core/name.h"
#include "handles/texture.h"
#include "handles/texture_view.h"
#include "handles/handle_vector.h"
#include "rhi/resources/texture.h"
#include "rhi/resources/texture_view.h"

namespace fe::renderer::rg
{

class Texture
{
public:
    Texture() = default;
    Texture(const TextureCreateInfo& createInfo, Name textureName);
    ~Texture();

    TextureRef handle() const { return m_handle; }

    TextureViewRef dsv() const;
    TextureViewRef srv() const;
    TextureViewRef rtv(uint32 mipLevel = 0) const;
    TextureViewRef uav(uint32 mipLevel = 0) const;

    uint32 dsv_descriptor() const;
    uint32 srv_descriptor() const;
    uint32 rtv_descriptor(uint32 mipLevel = 0) const;
    uint32 uav_descriptor(uint32 mipLevel = 0) const;

private:
    Name m_name;
    TextureHandle m_handle;
    mutable TextureViewHandle m_dsTextureView;
    mutable TextureViewHandle m_srTextureView;
    mutable HandleVector<TextureViewHandle> m_rtTextureViews;
    mutable HandleVector<TextureViewHandle> m_uaTextureViews;

    void reserve_texture_view_arrays();
    std::string get_view_name(const std::string& typeStr, uint32 mipLevel) const;
};

}