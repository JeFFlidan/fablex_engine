#include "render_surface.h"
#include "rhi/rhi.h"

namespace fe::renderer
{

void RenderSurface::set_default_viewport(rhi::CommandBuffer* cmd) const
{
    set_viewport(cmd, width, height);
}

void RenderSurface::set_default_scissor(rhi::CommandBuffer* cmd) const
{
    set_scissor(cmd, (int32)width, (int32)height);
}

void RenderSurface::set_viewport_by_window(rhi::CommandBuffer* cmd) const
{
    set_viewport(cmd, get_window_width(), get_window_height());
}

void RenderSurface::set_scissor_by_window(rhi::CommandBuffer* cmd) const
{
    set_scissor(cmd, (int32)get_window_width(), (int32)get_window_height());
}

void RenderSurface::set_viewport(rhi::CommandBuffer* cmd, uint32 inWidth, uint32 inHeight) const
{
    rhi::Viewport viewport;
    viewport.width = inWidth;
    viewport.height = inHeight;
    std::vector<rhi::Viewport> viewports = { viewport };
    rhi::set_viewports(cmd, viewports);
}

void RenderSurface::set_scissor(rhi::CommandBuffer* cmd, int32 inWidth, int32 inHeight) const
{
    rhi::Scissor scissor;
    scissor.right = inWidth;
    scissor.bottom = inHeight;
    std::vector<rhi::Scissor> scissors = { scissor };
    rhi::set_scissors(cmd, scissors);
}

}