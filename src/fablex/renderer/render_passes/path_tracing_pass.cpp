#pragma once

#include "path_tracing_pass.h"
#include "resource_scheduler.h"
#include "globals.h"

#include "rhi/rhi.h"
#include "scene_manager/scene_manager.h"
#include "engine/components/camera_component.h"

namespace fe::renderer
{

constexpr uint32 MAX_ACCUMULATION_FACTOR = 4096;

FE_DEFINE_OBJECT(PathTracingPass, RenderPass);

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
    RenderPass::schedule_resources();
    ResourceScheduler::use_ray_tracing(get_name());
}

void PathTracingPass::execute(rhi::CommandBuffer* cmd)
{
    ++m_accumulationFactor;
    if (m_accumulationFactor > MAX_ACCUMULATION_FACTOR)
        m_accumulationFactor = MAX_ACCUMULATION_FACTOR;

    set_default_viewport_and_scissor(cmd);
    bind_pipeline(cmd);

    PathTracingPushConstants pushConstants;
    fill_push_constants(pushConstants);

    pushConstants.tlas = m_renderContext->scene_manager()->scene_tlas()->descriptorIndex;
    pushConstants.bounceCount = 10;
    pushConstants.frameNumber = g_frameNumber;
    pushConstants.accumulationFactor = 1.0f / (m_accumulationFactor + 1.0f);
    pushConstants.alpha = 0.05f;
    pushConstants.momentsAlpha = 0.2f;

    push_constants(cmd, &pushConstants);
    
    rhi::DispatchRaysInfo info;
    info.width = m_renderContext->render_surface().width;
    info.height = m_renderContext->render_surface().height;
    info.depth = 1;

    fill_dispatch_rays_info(info);
    rhi::dispatch_rays(cmd, &info);
}

}