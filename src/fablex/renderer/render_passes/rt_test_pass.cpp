#include "rt_test_pass.h"
#include "resource_scheduler.h"
#include "render_context.h"
#include "scene_manager/scene_manager.h"

#include "rhi/rhi.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(RTTestPass, Object);

void RTTestPass::create_pipeline()
{
    create_ray_tracing_pipeline();
}

void RTTestPass::schedule_resources()
{
    RenderPass::schedule_resources();
    ResourceScheduler::use_ray_tracing(get_name());
}

void RTTestPass::execute(rhi::CommandBuffer* cmd)
{
    set_default_viewport_and_scissor(cmd);
    bind_pipeline(cmd);

    RayTracingPushConstants pushConstants;
    fill_push_constants(pushConstants);
    pushConstants.tlasIndex = m_renderContext->scene_manager()->scene_tlas()->descriptorIndex;

    push_constants(cmd, &pushConstants);

    rhi::DispatchRaysInfo info;
    info.width = m_renderContext->render_surface().width;
    info.height = m_renderContext->render_surface().height;
    info.depth = 1;

    fill_dispatch_rays_info(info);
    rhi::dispatch_rays(cmd, &info);
}

}