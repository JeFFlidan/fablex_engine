#pragma once

#include "handles/command_buffer.h"
#include "rhi/resources/swap_chain.h"

namespace fe::renderer
{

struct RenderSurface
{
    uint32 width;
    uint32 height;
    Format renderTargetFormat;
    Format depthStencilFormat;
    SwapChainRef mainSwapChain;

    uint32 get_window_width() const
    {
        return mainSwapChain->window->get_info().width;
    }

    uint32 get_window_height() const
    {
        return mainSwapChain->window->get_info().height;
    }

    void set_default_viewport(CommandBufferRef cmd) const;
    void set_default_scissor(CommandBufferRef cmd) const;
    void set_viewport_by_window(CommandBufferRef cmd) const;
    void set_scissor_by_window(CommandBufferRef cmd) const;
    void set_viewport(CommandBufferRef cmd, uint32 inWidth, uint32 inHeight) const;
    void set_scissor(CommandBufferRef cmd, int32 inWidth, int32 inHeight) const;
};    

}