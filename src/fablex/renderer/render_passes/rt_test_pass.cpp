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

void RTTestPass::execute(rhi::CommandBuffer* cmd)
{
    set_default_viewport_and_scissor(cmd);
    bind_pipeline(cmd);

    RayTracingPushConstants pushConstants;
    pushConstants.outputTargetIndex = get_output_storage_texture_descriptor(
        offsetof(RayTracingPushConstants, outputTargetIndex)
    );
    pushConstants.tlasIndex = m_renderContext->get_scene_manager()->get_scene_TLAS()->descriptorIndex;

    push_constants(cmd, &pushConstants);

    rhi::DispatchRaysInfo info;
    info.width = m_renderContext->get_render_surface().width;
    info.height = m_renderContext->get_render_surface().height;
    info.depth = 1;

    fill_dispatch_rays_info(info);
    rhi::dispatch_rays(cmd, &info);
}

void RTTestPass::schedule_resources_internal()
{
    ResourceScheduler::use_ray_tracing(get_name());
}

}