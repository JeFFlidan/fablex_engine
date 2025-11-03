#include "texture.h"
#include "utils.h"
#include "rhi/rhi.h"
#include "rhi/resources/texture.h"
#include "rhi/resources/texture_view.h"
#include "core/macro.h"

namespace fe::renderer::rg
{

Texture::Texture(const TextureCreateInfo& createInfo, Name textureName) 
    : m_name(textureName)
{
    m_handle.init(createInfo);
    m_handle.set_name(m_name.to_string());

    reserve_texture_view_arrays();
}

Texture::~Texture()
{

}

TextureViewRef Texture::dsv() const
{
    if (!m_dsTextureView)
    {
        m_dsTextureView.init(
            TextureViewCreateInfo
            {
                .texture = m_handle,
                .type = ViewType::DSV,
            }
        );

        m_dsTextureView.set_name(get_view_name("DSV", 0));
    }

    return m_dsTextureView;
}

TextureViewRef Texture::srv() const
{
    if (!m_srTextureView)
    {
        m_srTextureView.init(
            TextureViewCreateInfo
            {
                .texture = m_handle,
                .type = ViewType::SRV
            }
        );

        m_srTextureView.set_name(get_view_name("SRV", 0));
    }

    return m_srTextureView;
}

TextureViewRef Texture::rtv(uint32 mipLevel) const
{
    FE_CHECK_MSG(mipLevel < m_handle->mipLevels, "Requested RT texture view exceeds texture's amount of mip level.");

    if (!m_rtTextureViews[mipLevel])
    {
        m_rtTextureViews[mipLevel].init(
            TextureViewCreateInfo
            {
                .texture = m_handle,
                .baseMipLevel = mipLevel,
                .type = ViewType::RTV,
            }
        );

        m_rtTextureViews[mipLevel].set_name(get_view_name("RTV", mipLevel));
    }

    return m_rtTextureViews[mipLevel];
}

TextureViewRef Texture::uav(uint32 mipLevel) const
{
    FE_CHECK_MSG(mipLevel < m_handle->mipLevels, "Requested UA texture view exceeds texture's amount of mip level.");

    if (!m_uaTextureViews[mipLevel])
    {
        m_uaTextureViews[mipLevel].init(
            TextureViewCreateInfo
            {
                .texture = m_handle,
                .baseMipLevel = mipLevel,
                .type = ViewType::UAV
            }
        );

        m_uaTextureViews[mipLevel].set_name(get_view_name("UAV", mipLevel));
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
    for (uint32 i = 0; i != m_handle->mipLevels; ++i)
    {
        m_rtTextureViews.emplace();
        m_uaTextureViews.emplace();
    }
}

std::string Texture::get_view_name(const std::string& typeStr, uint32 mipLevel) const
{
    return Utils::create_resource_name("{}{}View-{}", m_name.to_string(), typeStr, mipLevel);
}

}