#include "triangle_pass.h"
#include "render_context.h"
#include "rhi/rhi.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(TrianglePass, RenderPass);

void TrianglePass::create_pipeline()
{
    create_graphics_pipeline();
}

void TrianglePass::execute(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd);

    rhi::Viewport viewport;
    viewport.width = m_renderContext->get_render_surface().width;
    viewport.height = m_renderContext->get_render_surface().height;
    std::vector<rhi::Viewport> viewports = { viewport };
    rhi::set_viewports(cmd, viewports);

    rhi::Scissor scissor;
    scissor.right = (int32_t)m_renderContext->get_render_surface().width;
    scissor.bottom = (int32_t)m_renderContext->get_render_surface().height;
    std::vector<rhi::Scissor> scissors = { scissor };
    rhi::set_scissors(cmd, scissors);

    bind_pipeline(cmd);

    rhi::draw(cmd, 3);
}

}