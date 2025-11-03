#include "rt_test_pass.h"
#include "render_context.h"
#include "scene_manager/scene_manager.h"
#include "render_graph/resource_scheduler.h"

#include "rhi/rhi.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(RTTestPass, rg::RenderPass);

void RTTestPass::create_pipeline()
{
    create_ray_tracing_pipeline();
}

void RTTestPass::schedule_resources()
{
    rg::RenderPass::schedule_resources();
    rg::ResourceScheduler::use_ray_tracing(name());
}

void RTTestPass::execute(CommandBufferRef cmd)
{
    set_default_viewport_and_scissor(cmd);
    bind_pipeline(cmd);

    RayTracingPushConstants pushConstants;
    fill_push_constants(pushConstants);
    pushConstants.tlas = m_renderContext->scene_manager()->tlas_descriptor();

    push_constants(cmd, &pushConstants);

    DispatchRaysInfo info;
    info.width = m_renderContext->render_surface().width;
    info.height = m_renderContext->render_surface().height;
    info.depth = 1;

    fill_dispatch_rays_info(info);
    cmd.dispatch_rays(&info);
}

}