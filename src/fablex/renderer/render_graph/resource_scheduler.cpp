#include "resource_scheduler.h"
#include "resource_manager.h"
#include "render_graph.h"
#include "globals.h"
#include "rhi/utils.h"
#include "rhi/resources/texture.h"

namespace fe::renderer::rg
{

void ResourceScheduler::init(const RenderContext* renderContext)
{
    s_renderContext = renderContext;
}

void ResourceScheduler::create_render_target(
    RenderPassName renderPassName,
    const TextureMetadata* textureMetadata,
    std::optional<ResourceName> customResourceName
)
{
    queue_resource_allocation(
        renderPassName, 
        textureMetadata, 
        customResourceName, 
        s_renderContext->render_surface().renderTargetFormat, 
        ResourceUsage::COLOR_ATTACHMENT,
        ResourceLayout::COLOR_ATTACHMENT
    );
}

void ResourceScheduler::create_depth_stencil(
    RenderPassName renderPassName,
    const TextureMetadata* textureMetadata,
    std::optional<ResourceName> customResourceName
)
{
    queue_resource_allocation(
        renderPassName, 
        textureMetadata, 
        customResourceName, 
        s_renderContext->render_surface().renderTargetFormat, 
        ResourceUsage::DEPTH_STENCIL_ATTACHMENT,
        ResourceLayout::DEPTH_STENCIL
    );
}

void ResourceScheduler::create_storage_texture(
    RenderPassName renderPassName,
    const TextureMetadata* textureMetadata,
    std::optional<ResourceName> customResourceName
)
{
    queue_resource_allocation(
        renderPassName, 
        textureMetadata, 
        customResourceName, 
        s_renderContext->render_surface().renderTargetFormat, 
        ResourceUsage::STORAGE_TEXTURE,
        ResourceLayout::GENERAL | ResourceLayout::SHADER_WRITE
    );
}

void ResourceScheduler::read_texture(RenderPassName renderPassName, ResourceName resourceName)
{
    s_renderContext->render_graph_resource_manager()->queue_resource_usage(
        renderPassName, 
        resourceName,
        [
            renderPassName,
            resourceName
        ](ResourceSchedulingInfo& schedulingInfo)
        {
            add_render_graph_read_dependency(renderPassName, resourceName, 1);
            update_view_infos(renderPassName, schedulingInfo, ResourceLayout::SHADER_READ, 1);
        } 
    );
}

void ResourceScheduler::read_previous_texture(
    RenderPassName renderPassName,
    const TextureMetadata* textureMetadata,
    std::optional<ResourceName> customResourceName
)
{
    queue_resource_allocation(
        renderPassName, 
        textureMetadata, 
        customResourceName, 
        s_renderContext->render_surface().renderTargetFormat, 
        ResourceUsage::STORAGE_TEXTURE,
        ResourceLayout::SHADER_READ
    );
}

void ResourceScheduler::write_to_back_buffer(RenderPassName renderPassName)
{
    RenderGraph::Node* node = s_renderContext->render_graph()->node(renderPassName);
    FE_CHECK(node);

    node->add_write_dependency(BACK_BUFFER_NAME, 1);
}

void ResourceScheduler::use_ray_tracing(RenderPassName renderPassName)
{
    RenderGraph::Node* node = s_renderContext->render_graph()->node(renderPassName);
    FE_CHECK(node);
    
    node->useRayTracing = true;
}

void ResourceScheduler::add_render_graph_read_dependency(
    RenderPassName renderPassName,
    ResourceName resourceName,
    uint32 mipCount
)
{
    RenderGraph::Node* node = s_renderContext->render_graph()->node(renderPassName);
    FE_CHECK(node);

    node->add_read_dependency(resourceName, mipCount);
}

void ResourceScheduler::add_render_graph_write_dependency(
    RenderPassName renderPassName,
    ResourceName resourceName,
    uint32 mipCount
)
{
    RenderGraph::Node* node = s_renderContext->render_graph()->node(renderPassName);
    FE_CHECK(node);

    node->add_write_dependency(resourceName, mipCount);
}

void ResourceScheduler::update_view_infos(
    RenderPassName renderPassName,
    ResourceSchedulingInfo& schedulingInfo,
    ResourceLayout layout,
    uint32 mipCount
)
{
    uint32 firstMip = 0;
    uint32 lastMip = mipCount - 1;

    for (uint32 mip = firstMip; mip <= lastMip; ++mip)
    {
        schedulingInfo.add_view_info(renderPassName, mip, layout);
    }
}

void ResourceScheduler::queue_resource_allocation(
    RenderPassName renderPassName,
    const TextureMetadata* textureMetadata,
    std::optional<ResourceName> customResourceName,
    Format format,
    ResourceUsage mainUsage,
    ResourceLayout initialLayout
)
{
    if (!textureMetadata && !customResourceName)
        FE_LOG(LogRenderer, FATAL, "No render target name!");

    TextureCreateInfo newTextureInfo;
    newTextureInfo.format = format;

    fill_info_from_metadata(mainUsage, textureMetadata, newTextureInfo);

    ResourceName resourceName = customResourceName ? *customResourceName : textureMetadata->name;

    s_renderContext->render_graph_resource_manager()->queue_resource_allocation(
        renderPassName,
        resourceName, 
        newTextureInfo, 
        [
            renderPassName,
            resourceName,
            initialLayout
        ](ResourceSchedulingInfo& schedulingInfo)
        {
            add_render_graph_write_dependency(renderPassName, resourceName, 1);
            update_view_infos(renderPassName, schedulingInfo, initialLayout, 1);
        }
    );
}

void ResourceScheduler::fill_info_from_metadata(
    ResourceUsage mainTextureUsage,
    const TextureMetadata* inMetadata,
    TextureCreateInfo& outInfo
)
{
    outInfo.textureUsage = 
        mainTextureUsage 
        | ResourceUsage::SAMPLED_TEXTURE 
        | ResourceUsage::TRANSFER_SRC;

    if (inMetadata)
    {
        outInfo.textureUsage |= inMetadata->has_flag(ResourceMetadataFlag::TRANSFER_DST) ? ResourceUsage::TRANSFER_DST : ResourceUsage::UNDEFINED;
    }

    outInfo.width = s_renderContext->render_surface().width;
    outInfo.height = s_renderContext->render_surface().height;
    outInfo.layersCount = inMetadata ? inMetadata->layerCount : 1;
    
    if (inMetadata && inMetadata->sampleCount != SampleCount::UNDEFINED)
        outInfo.samplesCount = inMetadata->sampleCount;
    else
        outInfo.samplesCount = SampleCount::BIT_1;

    if (inMetadata && inMetadata->format != Format::UNDEFINED)
        outInfo.format = inMetadata->format;

    outInfo.dimension = TextureDimension::TEXTURE2D;
}

}