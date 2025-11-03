#include "render_surface.h"
#include "rhi/rhi.h"
#include "rhi/resources/viewport.h"

namespace fe::renderer
{

void RenderSurface::set_default_viewport(CommandBufferRef cmd) const
{
    set_viewport(cmd, width, height);
}

void RenderSurface::set_default_scissor(CommandBufferRef cmd) const
{
    set_scissor(cmd, (int32)width, (int32)height);
}

void RenderSurface::set_viewport_by_window(CommandBufferRef cmd) const
{
    set_viewport(cmd, get_window_width(), get_window_height());
}

void RenderSurface::set_scissor_by_window(CommandBufferRef cmd) const
{
    set_scissor(cmd, (int32)get_window_width(), (int32)get_window_height());
}

void RenderSurface::set_viewport(CommandBufferRef cmd, uint32 inWidth, uint32 inHeight) const
{
    Viewport viewport;
    viewport.width = inWidth;
    viewport.height = inHeight;
    cmd.set_viewports({ viewport });
}

void RenderSurface::set_scissor(CommandBufferRef cmd, int32 inWidth, int32 inHeight) const
{
    Scissor scissor;
    scissor.right = inWidth;
    scissor.bottom = inHeight;
    cmd.set_scissors({ scissor });
}

}