#pragma once

#include "path_tracing_pass.h"
#include "render_graph/resource_scheduler.h"
#include "render_graph/resource_manager.h"
#include "globals.h"

#include "rhi/rhi.h"
#include "scene_manager/scene_manager.h"
#include "engine/components/camera_component.h"
#include "shaders/interops/shader_interop_push_constants.h"

namespace fe::renderer
{

constexpr uint32 MAX_ACCUMULATION_FACTOR = 4096;

FE_DEFINE_OBJECT(PathTracingPass, rg::RenderPass);

PathTracingPass::PathTracingPass()
{
    EventManager::subscribe<engine::CameraMovedEvent>([this](const engine::CameraMovedEvent&)
    {
        m_accumulationFactor = 0;
    });
}

void PathTracingPass::create_pipeline()
{
    create_ray_tracing_pipeline();
}

void PathTracingPass::schedule_resources()
{
    rg::RenderPass::schedule_resources();
    rg::ResourceScheduler::use_ray_tracing(name());
}

void PathTracingPass::execute(CommandBufferRef cmd)
{
    rg::ResourceManager* resourceManager = m_renderContext->render_graph_resource_manager();
    rg::Resource* finalIllumination = resourceManager->get_resource("FilteredIllumination0");
    FE_CHECK(finalIllumination);

    ++m_accumulationFactor;
    if (m_accumulationFactor > MAX_ACCUMULATION_FACTOR)
        m_accumulationFactor = MAX_ACCUMULATION_FACTOR;

    set_default_viewport_and_scissor(cmd);
    bind_pipeline(cmd);

    PathTracingPushConstants pushConstants;
    fill_push_constants(pushConstants);

    pushConstants.tlas = m_renderContext->scene_manager()->tlas_descriptor();
    pushConstants.bounceCount = 10;
    pushConstants.frameNumber = g_frameNumber;
    pushConstants.accumulationFactor = m_accumulationFactor;
    pushConstants.alpha = 0.05f;
    pushConstants.momentsAlpha = 0.2f;
    pushConstants.inPrevIllumination = finalIllumination->texture().srv_descriptor();

    push_constants(cmd, &pushConstants);
    
    DispatchRaysInfo info;
    info.width = m_renderContext->render_surface().width;
    info.height = m_renderContext->render_surface().height;
    info.depth = 1;

    fill_dispatch_rays_info(info);
    cmd.dispatch_rays(&info);
}

}