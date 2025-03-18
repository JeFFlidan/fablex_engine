#include "render_pass.h"
#include "render_context.h"
#include "render_graph_metadata.h"
#include "render_graph.h"
#include "render_graph_resource_manager.h"
#include "resource_scheduler.h"
#include "rhi/utils.h"

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
        ResourceScheduler::read_texture(get_name(), textureName);

    for (const RenderTargetMetadata& renderTargetMetadata : m_metadata->renderTargetsMetadata)
    {
        if (renderTargetMetadata.textureName.is_valid())
        {     
            const TextureMetadata& textureMetadata = get_texture_metadata(renderTargetMetadata.textureName);
            rhi::TextureInfo info;
            fill_texture_info(textureMetadata, info);
            
            if (rhi::is_depth_stencil_format(textureMetadata.format))
                ResourceScheduler::create_depth_stencil(get_name(), textureMetadata.textureName);
            else
                ResourceScheduler::create_render_target(get_name(), textureMetadata.textureName, &info);
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
        ResourceScheduler::create_storage_texture(get_name(), textureName, &info);
    }

    schedule_resources_internal();
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
    RenderGraphResourceManager* resourceManager = m_renderContext->get_render_graph_resource_manager();

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

uint32 RenderPass::get_input_texture_descriptor(uint64 pushConstantsOffset, rhi::ViewType viewType, uint32 mipLevel) const
{
    ResourceName textureName = m_metadata->inputTextureNames.at(pushConstantsOffset / 4);
    RenderPassName renderPassName = m_metadata->renderPassName;
    RenderGraphResourceManager* resourceManager = m_renderContext->get_render_graph_resource_manager();

    switch (viewType)
    {
    case rhi::ViewType::DSV:
        return resourceManager->get_dsv_desciptor(renderPassName, textureName);
    case rhi::ViewType::RTV:
        return resourceManager->get_rtv_descriptor(renderPassName, textureName, mipLevel);
    case rhi::ViewType::SRV:
        return resourceManager->get_texture_srv_descriptor(renderPassName, textureName, mipLevel);
    case rhi::ViewType::UAV:
        return resourceManager->get_texture_uav_descriptor(renderPassName, textureName, mipLevel);
    default:
        FE_CHECK(0);
        return 0;
    }
}

uint32 RenderPass::get_sampler_descriptor(ResourceName samplerName) const
{
    return m_renderContext->get_render_graph_resource_manager()->get_sampler_descriptor(samplerName);
}

void RenderPass::create_compute_pipeline()
{
    m_renderContext->get_pipeline_manager()->create_compute_pipeline(get_pipeline_metadata());
}
    
void RenderPass::create_graphics_pipeline()
{
    m_renderContext->get_pipeline_manager()->create_graphics_pipeline(get_pipeline_metadata());
}

void RenderPass::create_graphics_pipeline(const PipelineManager::GraphicsPipelineConfigurator& configurator)
{
    m_renderContext->get_pipeline_manager()->create_graphics_pipeline(get_pipeline_metadata(), configurator);
}

void RenderPass::create_ray_tracing_pipeline()
{
    m_renderContext->get_pipeline_manager()->create_ray_tracing_pipeline(get_pipeline_metadata());
}

void RenderPass::create_ray_tracing_pipeline(const PipelineManager::RayTracingPipelineConfigurator& configurator)
{
    m_renderContext->get_pipeline_manager()->create_ray_tracing_pipeline(get_pipeline_metadata(), configurator);
}

void RenderPass::bind_pipeline(rhi::CommandBuffer* cmd)
{
    m_renderContext->get_pipeline_manager()->bind_pipeline(cmd, m_metadata->pipelineName);
}

void RenderPass::push_constants(rhi::CommandBuffer* cmd, void* data)
{
    m_renderContext->get_pipeline_manager()->push_constants(cmd, m_metadata->pipelineName, data);
}

const PipelineMetadata& RenderPass::get_pipeline_metadata() const
{
    const RenderGraphMetadata* renderGraphMetadata = m_renderContext->get_render_graph()->get_metadata();
    const PipelineMetadata* pipelineMetadata = renderGraphMetadata->get_pipeline_metadata(m_metadata->pipelineName);
    if (!pipelineMetadata)
        FE_LOG(LogRenderer, FATAL, "No pipeline metadata for name {}", m_metadata->pipelineName);
    return *pipelineMetadata;
}

const TextureMetadata& RenderPass::get_texture_metadata(ResourceName textureName) const
{
    FE_CHECK(textureName.is_valid());

    const RenderGraphMetadata* renderGraphMetadata = m_renderContext->get_render_graph()->get_metadata();
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

}