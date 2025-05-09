#include "render_pass.h"
#include "render_context.h"
#include "render_graph_metadata.h"
#include "render_graph.h"
#include "render_graph_resource_manager.h"
#include "resource_scheduler.h"
#include "globals.h"
#include "rhi/utils.h"
#include "rhi/rhi.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(RenderPass, Object)

void RenderPass::init(const RenderPassMetadata& metadata, const RenderContext* renderContext)
{
    FE_CHECK(renderContext);
    m_renderContext = renderContext;
    m_metadata = &metadata;
}

void RenderPass::schedule_resources()
{
    FE_CHECK(m_metadata);

    for (ResourceName textureName : m_metadata->inputTextureNames)
    {
        const TextureMetadata& textureMetadata = get_texture_metadata(textureName);

        if (textureMetadata.crossFrameRead)
        {
            auto [prevFrameName, curFrameName] = get_resource_names_xfr(textureMetadata.textureName);
            ResourceScheduler::read_texture(get_name(), prevFrameName);
            ResourceScheduler::read_texture(get_name(), curFrameName);
        }
        else
        {
            ResourceScheduler::read_texture(get_name(), textureName);
        }
    }

    for (const RenderTargetMetadata& renderTargetMetadata : m_metadata->renderTargetsMetadata)
    {
        if (renderTargetMetadata.textureName.is_valid())
        {     
            const TextureMetadata& textureMetadata = get_texture_metadata(renderTargetMetadata.textureName);
            rhi::TextureInfo info;
            fill_texture_info(textureMetadata, info);
            
            if (textureMetadata.crossFrameRead)
            {
                auto [prevFrameName, curFrameName] = get_resource_names_xfr(textureMetadata.textureName);
                if (rhi::is_depth_stencil_format(textureMetadata.format))
                {
                    ResourceScheduler::create_depth_stencil(get_name(), prevFrameName, &info);
                    ResourceScheduler::create_depth_stencil(get_name(), curFrameName, &info);
                }
                else
                {
                    ResourceScheduler::create_render_target(get_name(), prevFrameName, &info);
                    ResourceScheduler::create_render_target(get_name(), curFrameName, &info);
                }
            }
            else
            {
                if (rhi::is_depth_stencil_format(textureMetadata.format))
                {
                    ResourceScheduler::create_depth_stencil(get_name(), textureMetadata.textureName, &info);
                }
                else
                {
                    ResourceScheduler::create_render_target(get_name(), textureMetadata.textureName, &info);
                }
            }
        }
        else
        {
            ResourceScheduler::write_to_back_buffer(get_name());
        }
    }

    for (ResourceName textureName : m_metadata->outputStorageTextureNames)
    {
        const TextureMetadata& textureMetadata = get_texture_metadata(textureName);
        rhi::TextureInfo info;
        fill_texture_info(textureMetadata, info);

        if (textureMetadata.crossFrameRead)
        {
            auto [prevFrameName, curFrameName] = get_resource_names_xfr(textureName);
            ResourceScheduler::create_storage_texture(get_name(), prevFrameName, &info);
            ResourceScheduler::create_storage_texture(get_name(), curFrameName, &info);
        }
        else
        {
            ResourceScheduler::create_storage_texture(get_name(), textureName, &info);
        }
    }
}

RenderPassInfo RenderPass::get_info() const
{
    return RenderPassInfo(
        m_metadata->renderPassName,
        m_metadata->pipelineName,
        m_metadata->type
    );
}

void RenderPass::fill_rendering_begin_info(rhi::RenderingBeginInfo& outBeginInfo) const
{
    RenderGraphResourceManager* resourceManager = m_renderContext->render_graph_resource_manager();

    switch (outBeginInfo.type)
    {
    case rhi::RenderingBeginInfo::OFFSCREEN_PASS:
    {
        for (const RenderTargetMetadata& renderTargetMetadata : m_metadata->renderTargetsMetadata)
        {
            Texture& texture = resourceManager->get_resource(renderTargetMetadata.textureName)->get_texture();
            FE_CHECK(texture.get_handle());
            
            const TextureMetadata& textureMetadata = get_texture_metadata(renderTargetMetadata.textureName);

            rhi::RenderTarget& renderTarget = outBeginInfo.offscreenPass.renderTargets.emplace_back();
            
            if (rhi::is_depth_stencil_format(textureMetadata.format))
                renderTarget.target = texture.get_dsv();
            else
                renderTarget.target = texture.get_rtv();

            FE_CHECK(renderTarget.target);

            renderTarget.clearValue = renderTargetMetadata.clearValues;
            renderTarget.loadOp = renderTargetMetadata.loadOp;
            renderTarget.storeOp = renderTargetMetadata.storeOp;
        }

        break;
    }
    case rhi::RenderingBeginInfo::SWAP_CHAIN_PASS:
    {
        outBeginInfo.swapChainPass.clearValues = m_metadata->renderTargetsMetadata.at(0).clearValues;
        break;
    }
    }
}

void RenderPass::create_compute_pipeline()
{
    m_renderContext->pipeline_manager()->create_compute_pipeline(get_pipeline_metadata());
}
    
void RenderPass::create_graphics_pipeline()
{
    m_renderContext->pipeline_manager()->create_graphics_pipeline(get_pipeline_metadata());
}

void RenderPass::create_graphics_pipeline(const PipelineManager::GraphicsPipelineConfigurator& configurator)
{
    m_renderContext->pipeline_manager()->create_graphics_pipeline(get_pipeline_metadata(), configurator);
}

void RenderPass::create_ray_tracing_pipeline()
{
    m_renderContext->pipeline_manager()->create_ray_tracing_pipeline(get_pipeline_metadata());
}

void RenderPass::create_ray_tracing_pipeline(const PipelineManager::RayTracingPipelineConfigurator& configurator)
{
    m_renderContext->pipeline_manager()->create_ray_tracing_pipeline(get_pipeline_metadata(), configurator);
}

SceneManager* RenderPass::scene_manager() const
{
    return m_renderContext->scene_manager();
}

void RenderPass::bind_pipeline(rhi::CommandBuffer* cmd)
{
    m_renderContext->pipeline_manager()->bind_pipeline(cmd, m_metadata->pipelineName);
}

void RenderPass::push_constants(rhi::CommandBuffer* cmd, void* data)
{
    m_renderContext->pipeline_manager()->push_constants(cmd, m_metadata->pipelineName, data);
}

void RenderPass::set_default_viewport_and_scissor(rhi::CommandBuffer* cmd) const
{
    m_renderContext->render_surface().set_default_viewport(cmd);
    m_renderContext->render_surface().set_default_scissor(cmd);
}

void RenderPass::set_viewport_and_scissor_by_window(rhi::CommandBuffer* cmd) const
{
    m_renderContext->render_surface().set_viewport_by_window(cmd);
    m_renderContext->render_surface().set_scissor_by_window(cmd);
}

void RenderPass::fill_dispatch_rays_info(rhi::DispatchRaysInfo& outInfo) const
{
    m_renderContext->pipeline_manager()->fill_dispatch_rays_info(m_metadata->pipelineName, outInfo);
}

DispatchSizes dispatch_group_count(const DispatchSizes& threadCounts, const DispatchSizes& groupSizes)
{
    uint32 x = std::max(ceilf((float)threadCounts[0] / groupSizes[0]), 1.0f);
    uint32 y = std::max(ceilf((float)threadCounts[1] / groupSizes[1]), 1.0f);
    uint32 z = std::max(ceilf((float)threadCounts[2] / groupSizes[2]), 1.0f);

    return {x, y, z};
}

void RenderPass::dispatch(rhi::CommandBuffer* cmd, const RenderSurface& surface, const DispatchSizes& groupSizes)
{
    DispatchSizes groupCount = dispatch_group_count({surface.width, surface.height, 1}, groupSizes);
    rhi::dispatch(cmd, groupCount[0], groupCount[1], groupCount[2]);
}

const RenderGraphMetadata& RenderPass::get_render_graph_metadata() const
{
    return *m_renderContext->render_graph()->get_metadata();
}

const PipelineMetadata& RenderPass::get_pipeline_metadata() const
{
    const RenderGraphMetadata* renderGraphMetadata = m_renderContext->render_graph()->get_metadata();
    const PipelineMetadata* pipelineMetadata = renderGraphMetadata->get_pipeline_metadata(m_metadata->pipelineName);
    if (!pipelineMetadata)
        FE_LOG(LogRenderer, FATAL, "No pipeline metadata for name {}", m_metadata->pipelineName);
    return *pipelineMetadata;
}

const TextureMetadata& RenderPass::get_texture_metadata(ResourceName textureName) const
{
    FE_CHECK(textureName.is_valid());

    const RenderGraphMetadata* renderGraphMetadata = m_renderContext->render_graph()->get_metadata();
    const TextureMetadata* textureMetadata = renderGraphMetadata->get_texture_metadata(textureName);
    if (!textureMetadata)
        FE_LOG(LogRenderer, FATAL, "No metadata for resource with name {}", textureName);

    return *textureMetadata;
}

void RenderPass::fill_texture_info(const TextureMetadata& inMetadata, rhi::TextureInfo& outInfo) const
{
    outInfo.layersCount = inMetadata.layerCount;
    outInfo.samplesCount = inMetadata.sampleCount;
    outInfo.format = inMetadata.format;
    
    if (inMetadata.useMips)
    {
        // TODO: How to request mips creation?
    }

    if (inMetadata.isTransferDst)
        outInfo.textureUsage |= rhi::ResourceUsage::TRANSFER_DST;
}

RenderPass::ResourceNamesXFR RenderPass::get_resource_names_xfr(ResourceName baseName) const
{
    return ResourceNamesXFR{
        get_prev_frame_resource_name(baseName),
        get_curr_frame_resource_name(baseName)
    };
}

ResourceName RenderPass::get_prev_frame_resource_name(ResourceName baseName) const
{
    return ResourceName(baseName.to_string() + std::to_string(get_prev_frame_index()));
}

ResourceName RenderPass::get_curr_frame_resource_name(ResourceName baseName) const
{
    return ResourceName(baseName.to_string() + std::to_string(get_curr_frame_index()));
}

void RenderPass::fill_push_constants(PushConstantsName pushConstantsName, void* data) const
{
    const RenderGraphMetadata& renderGraphMetadata = get_render_graph_metadata();
    const PushConstantsMetadata* pushConstantsMetadata = renderGraphMetadata.get_push_constants_metadata(pushConstantsName);

    if (!pushConstantsMetadata)
    {
        FE_LOG(LogRenderer, WARNING, "No push constants metadata with name {}", pushConstantsName);
        return;
    }

    uint8* typedData = static_cast<uint8*>(data);
    uint64 offset = 0;

    RenderGraphResourceManager* resourceManager = m_renderContext->render_graph_resource_manager();

    for (const auto& resourceMetadata : pushConstantsMetadata->resourcesMetadata)
    {
        const TextureMetadata* textureMetadata = renderGraphMetadata.get_texture_metadata(resourceMetadata.name);
        if (!textureMetadata)
            FE_LOG(LogRenderer, FATAL, "No texture metadata with name {}", resourceMetadata.name);

        ResourceName resourceName = resourceMetadata.name;

        if (textureMetadata->crossFrameRead)
        {
            if (resourceMetadata.previousFrame)
                resourceName = get_prev_frame_resource_name(resourceName);
            else
                resourceName = get_curr_frame_resource_name(resourceName);
        }

        uint32 descriptor = ~0u;

        if (resourceMetadata.write)
            descriptor = resourceManager->get_texture_uav_descriptor(get_name(), resourceName);
        else
            descriptor = resourceManager->get_texture_srv_descriptor(get_name(), resourceName);

        memcpy(typedData + offset, &descriptor, sizeof(uint32));
        offset += sizeof(uint32);
    }
}

}