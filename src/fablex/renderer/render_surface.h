#pragma once

#include "rhi/resources.h"

namespace fe::renderer
{

struct RenderSurface
{
    uint32 width;
    uint32 height;
    rhi::Format renderTargetFormat;
    rhi::Format depthStencilFormat;
    rhi::SwapChain* mainSwapChain;

    uint32 get_window_width() const
    {
        return mainSwapChain->window->get_info().width;
    }

    uint32 get_window_height() const
    {
        return mainSwapChain->window->get_info().height;
    }

    void set_default_viewport(rhi::CommandBuffer* cmd) const;
    void set_default_scissor(rhi::CommandBuffer* cmd) const;
    void set_viewport_by_window(rhi::CommandBuffer* cmd) const;
    void set_scissor_by_window(rhi::CommandBuffer* cmd) const;
    void set_viewport(rhi::CommandBuffer* cmd, uint32 inWidth, uint32 inHeight) const;
    void set_scissor(rhi::CommandBuffer* cmd, int32 inWidth, int32 inHeight) const;
};    

}