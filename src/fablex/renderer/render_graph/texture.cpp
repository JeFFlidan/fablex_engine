#include "texture.h"
#include "utils.h"
#include "rhi/rhi.h"
#include "core/macro.h"

namespace fe::renderer
{

Texture::Texture(rhi::Texture* handle, Name textureName) 
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
        rhi::TextureView* rtv = m_rtTextureViews[i];
        rhi::TextureView* uav = m_uaTextureViews[i];

        if (rtv) rhi::destroy_texture_view(rtv);
        if (uav) rhi::destroy_texture_view(uav);
    }

    if (m_handle) rhi::destroy_texture(m_handle);
}

rhi::TextureView* Texture::dsv() const
{
    if (!m_dsTextureView)
    {
        rhi::TextureViewInfo info;
        info.type = rhi::ViewType::DSV;
        info.texture = m_handle;
        rhi::create_texture_view(&m_dsTextureView, &info);
        rhi::set_name(m_handle, get_view_name(0));
    }

    return m_dsTextureView;
}

rhi::TextureView* Texture::srv() const
{
    if (!m_srTextureView)
    {
        rhi::TextureViewInfo info;
        info.type = rhi::ViewType::SRV;
        info.texture = m_handle;
        rhi::create_texture_view(&m_srTextureView, &info);
        rhi::set_name(m_handle, get_view_name(0));
    }

    return m_srTextureView;
}

rhi::TextureView* Texture::rtv(uint32 mipLevel) const
{
    FE_CHECK_MSG(mipLevel < m_handle->mipLevels, "Requested RT texture view exceeds texture's amount of mip level.");

    if (!m_rtTextureViews[mipLevel])
    {
        rhi::TextureViewInfo info;
        info.baseMipLevel = mipLevel;
        info.type = rhi::ViewType::RTV;
        info.texture = m_handle;
        rhi::create_texture_view(&m_rtTextureViews[mipLevel], &info);
        rhi::set_name(m_handle, get_view_name(mipLevel));
    }

    return m_rtTextureViews[mipLevel];
}

rhi::TextureView* Texture::uav(uint32 mipLevel) const
{
    FE_CHECK_MSG(mipLevel < m_handle->mipLevels, "Requested UA texture view exceeds texture's amount of mip level.");

    if (!m_uaTextureViews[mipLevel])
    {
        rhi::TextureViewInfo info;
        info.baseMipLevel = mipLevel;
        info.type = rhi::ViewType::UAV;
        info.texture = m_handle;
        rhi::create_texture_view(&m_uaTextureViews[mipLevel], &info);
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