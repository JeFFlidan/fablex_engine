#pragma once

#include "handles/swap_chain.h"
#include "rhi/resources/render_pass.h"

namespace fe::renderer::rg
{

class Texture;
struct TextureMetadata;
struct RenderTargetMetadata;

class RenderingInfoBase
{
public:
    using RenderingType = RenderingBeginInfo::Type;

    RenderingInfoBase(RenderingType type) : m_info(type) { }

    void enable_multiview(uint32 viewCount)
    {
        m_info.multiviewInfo.isEnabled = true;
        m_info.multiviewInfo.viewCount = viewCount;
    }

    void set_flags(RenderingBeginInfoFlags flags)
    {
        m_info.flags = flags;
    }

    RenderingType type() const
    {
        return m_info.type;
    }

    operator RenderingBeginInfo*()
    {
        return &m_info;
    }

protected:
    RenderingBeginInfo m_info;
};

class OffscreenPassRenderingInfo : public RenderingInfoBase
{
public:
    OffscreenPassRenderingInfo() 
        : RenderingInfoBase(RenderingType::OFFSCREEN_PASS) { }

    void add_render_target(
        const Texture& texture, 
        const TextureMetadata& textureMetadata,
        const RenderTargetMetadata& renderTargetMetadata
    );
};

class SwapChainPassRenderingInfo : public RenderingInfoBase
{
public:
    SwapChainPassRenderingInfo(SwapChainRef swapChain)
        : RenderingInfoBase(RenderingType::SWAP_CHAIN_PASS)
    {
        m_info.swapChainPass.swapChain = swapChain;
    }

    void set_swap_chain(SwapChainRef swapChain)
    {
        m_info.swapChainPass.swapChain = swapChain;
    }

    void set_render_target(const RenderTargetMetadata& renderTargetMetadata);
};

}