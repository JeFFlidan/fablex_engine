#include "texture.h"
#include "utils.h"
#include "rhi/rhi.h"
#include "core/macro.h"

namespace fe::renderer
{

Texture::Texture(rhi::TextureHandle handle, Name textureName) 
    : m_name(textureName), m_handle(handle) 
{
    reserve_texture_view_arrays();
    rhi::set_name(m_handle, m_name.to_string().c_str());
}

Texture::~Texture()
{
    if (m_dsTextureView) rhi::destroy_texture_view(m_dsTextureView);
    if (m_srTextureView) rhi::destroy_texture_view(m_srTextureView);

    for (uint32 i = 0; i < m_handle->mipLevels; ++i)
    {
        rhi::TextureViewHandle rtv = m_rtTextureViews[i];
        rhi::TextureViewHandle uav = m_uaTextureViews[i];

        if (rtv) rhi::destroy_texture_view(rtv);
        if (uav) rhi::destroy_texture_view(uav);
    }

    if (m_handle) rhi::destroy_texture(m_handle);
}

rhi::TextureViewHandle Texture::dsv() const
{
    if (!m_dsTextureView)
    {
        rhi::TextureViewInfo info;
        info.type = rhi::ViewType::DSV;
        rhi::create_texture_view(&m_dsTextureView, &info, m_handle);
        rhi::set_name(m_handle, get_view_name(0));
    }

    return m_dsTextureView;
}

rhi::TextureViewHandle Texture::srv() const
{
    if (!m_srTextureView)
    {
        rhi::TextureViewInfo info;
        info.type = rhi::ViewType::SRV;
        rhi::create_texture_view(&m_srTextureView, &info, m_handle);
        rhi::set_name(m_handle, get_view_name(0));
    }

    return m_srTextureView;
}

rhi::TextureViewHandle Texture::rtv(uint32 mipLevel) const
{
    FE_CHECK_MSG(mipLevel < m_handle->mipLevels, "Requested RT texture view exceeds texture's amount of mip level.");

    if (!m_rtTextureViews[mipLevel])
    {
        rhi::TextureViewInfo info;
        info.baseMipLevel = mipLevel;
        info.type = rhi::ViewType::RTV;
        rhi::create_texture_view(&m_rtTextureViews[mipLevel], &info, m_handle);
        rhi::set_name(m_handle, get_view_name(mipLevel));
    }

    return m_rtTextureViews[mipLevel];
}

rhi::TextureViewHandle Texture::uav(uint32 mipLevel) const
{
    FE_CHECK_MSG(mipLevel < m_handle->mipLevels, "Requested UA texture view exceeds texture's amount of mip level.");

    if (!m_uaTextureViews[mipLevel])
    {
        rhi::TextureViewInfo info;
        info.baseMipLevel = mipLevel;
        info.type = rhi::ViewType::UAV;
        rhi::create_texture_view(&m_uaTextureViews[mipLevel], &info, m_handle);
        rhi::set_name(m_handle, get_view_name(mipLevel));
    }

    return m_uaTextureViews[mipLevel];
}

uint32 Texture::dsv_descriptor() const
{
    return dsv()->descriptorIndex;
}

uint32 Texture::srv_descriptor() const
{
    return srv()->descriptorIndex;
}

uint32 Texture::rtv_descriptor(uint32 mipLevel) const
{
    return rtv(mipLevel)->descriptorIndex;
}

uint32 Texture::uav_descriptor(uint32 mipLevel) const
{
    return uav(mipLevel)->descriptorIndex;
}

void Texture::reserve_texture_view_arrays()
{
    m_rtTextureViews.resize(m_handle->mipLevels, nullptr);
    m_uaTextureViews.resize(m_handle->mipLevels, nullptr);
}

std::string Texture::get_view_name(uint32 mipLevel) const
{
    return Utils::create_resource_name("{}View-{}", m_name.to_string(), mipLevel);
}

}