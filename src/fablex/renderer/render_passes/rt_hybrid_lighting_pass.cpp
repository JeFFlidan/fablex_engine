#include "rt_hybrid_lighting_pass.h"
#include "render_context.h"
#include "scene_manager/scene_manager.h"
#include "render_graph/resource_scheduler.h"

#include "rhi/rhi.h"
#include "shaders/interops/shader_interop_push_constants.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(RTHybridLightingPass, rg::RenderPass);

void RTHybridLightingPass::create_pipeline()
{
    create_ray_tracing_pipeline();
}

void RTHybridLightingPass::schedule_resources()
{
    rg::RenderPass::schedule_resources();
    rg::ResourceScheduler::use_ray_tracing(name());
}

void RTHybridLightingPass::execute(CommandBufferRef cmd)
{
    set_default_viewport_and_scissor(cmd);
    bind_pipeline(cmd);

    RTHybridLightingPushConstants pushConstants;
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