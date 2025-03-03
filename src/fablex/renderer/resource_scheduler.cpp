#include "resource_scheduler.h"
#include "resource_manager.h"
#include "render_graph.h"

namespace fe::renderer
{

void ResourceScheduler::init(const RenderContext* renderContext)
{
    s_renderContext = renderContext;
}

void ResourceScheduler::create_render_target(
    RenderPassName renderPassName, 
    ResourceName resourceName,
    const rhi::TextureInfo* textureInfo
)
{
    rhi::TextureInfo newTextureInfo;
    newTextureInfo.format = s_renderContext->get_render_surface().renderTargetFormat;
    newTextureInfo.textureUsage = rhi::ResourceUsage::COLOR_ATTACHMENT | rhi::ResourceUsage::SAMPLED_TEXTURE | rhi::ResourceUsage::TRANSFER_SRC;
    newTextureInfo.width = s_renderContext->get_render_surface().width;
    newTextureInfo.height = s_renderContext->get_render_surface().height;
    newTextureInfo.dimension = rhi::TextureDimension::TEXTURE2D;

    fill_info_from_base(newTextureInfo, textureInfo);

    s_renderContext->get_resource_manager()->queue_resource_allocation(
        renderPassName,
        resourceName, 
        newTextureInfo, 
        [
            renderPassName,
            resourceName
        ](ResourceSchedulingInfo& schedulingInfo)
        {
            add_render_graph_write_dependency(renderPassName, resourceName, 1);
            update_view_infos(renderPassName, schedulingInfo, rhi::ResourceLayout::COLOR_ATTACHMENT, 1);
        }
    );
}

void ResourceScheduler::create_depth_stencil(
    RenderPassName renderPassName, 
    ResourceName resourceName, 
    const rhi::TextureInfo* textureInfo
)
{
    rhi::TextureInfo newTextureInfo;
    newTextureInfo.format = s_renderContext->get_render_surface().depthStencilFormat;
    newTextureInfo.textureUsage = rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT | rhi::ResourceUsage::SAMPLED_TEXTURE | rhi::ResourceUsage::TRANSFER_SRC;
    newTextureInfo.width = s_renderContext->get_render_surface().width;
    newTextureInfo.height = s_renderContext->get_render_surface().height;
    newTextureInfo.dimension = rhi::TextureDimension::TEXTURE2D;

    fill_info_from_base(newTextureInfo, textureInfo);

    s_renderContext->get_resource_manager()->queue_resource_allocation(
        renderPassName, 
        resourceName, 
        newTextureInfo,
        [
            renderPassName,
            resourceName
        ](ResourceSchedulingInfo& schedulingInfo)
        {
            add_render_graph_write_dependency(renderPassName, resourceName, 1);
            update_view_infos(renderPassName, schedulingInfo, rhi::ResourceLayout::DEPTH_STENCIL, 1);
        }
    );
}

void ResourceScheduler::create_storage_texture(
    RenderPassName renderPassName, 
    ResourceName resourceName,
    const rhi::TextureInfo* textureInfo
)
{
    rhi::TextureInfo newTextureInfo;
    newTextureInfo.format = rhi::Format::R32_SFLOAT;
    newTextureInfo.textureUsage = rhi::ResourceUsage::STORAGE_TEXTURE | rhi::ResourceUsage::SAMPLED_TEXTURE;
    newTextureInfo.width = s_renderContext->get_render_surface().width;
    newTextureInfo.height = s_renderContext->get_render_surface().height;
    newTextureInfo.dimension = rhi::TextureDimension::TEXTURE2D;

    fill_info_from_base(newTextureInfo, textureInfo);

    s_renderContext->get_resource_manager()->queue_resource_allocation(
        renderPassName, 
        resourceName, 
        newTextureInfo,
        [
            renderPassName,
            resourceName
        ](ResourceSchedulingInfo& schedulingInfo)
        {
            add_render_graph_write_dependency(renderPassName, resourceName, 1);
            update_view_infos(renderPassName, schedulingInfo, rhi::ResourceLayout::GENERAL, 1);
        }
    );
}

void ResourceScheduler::read_texture(ResourceName resourceName, RenderPassName renderPassName)
{
    s_renderContext->get_resource_manager()->queue_resource_usage(
        renderPassName, 
        resourceName,
        [
            renderPassName,
            resourceName
        ](ResourceSchedulingInfo& schedulingInfo)
        {
            add_render_graph_read_dependency(renderPassName, resourceName, 1);
            update_view_infos(renderPassName, schedulingInfo, rhi::ResourceLayout::SHADER_READ, 1);
        } 
    );
}

void ResourceScheduler::add_render_graph_read_dependency(
    RenderPassName renderPassName,
    ResourceName resourceName,
    uint32 mipCount
)
{
    RenderGraph::Node* node = s_renderContext->get_render_graph()->get_node(renderPassName);
    FE_CHECK(node);

    node->add_read_dependency(resourceName, mipCount);
}

void ResourceScheduler::add_render_graph_write_dependency(
    RenderPassName renderPassName,
    ResourceName resourceName,
    uint32 mipCount
)
{
    RenderGraph::Node* node = s_renderContext->get_render_graph()->get_node(renderPassName);
    FE_CHECK(node);

    node->add_write_dependency(resourceName, mipCount);
}

void ResourceScheduler::update_view_infos(
    RenderPassName renderPassName,
    ResourceSchedulingInfo& schedulingInfo,
    rhi::ResourceLayout layout,
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

void ResourceScheduler::fill_info_from_base(rhi::TextureInfo& outInfo, const rhi::TextureInfo* baseInfo)
{
    if (!baseInfo)
        return;

    if (baseInfo->width != 0)
        outInfo.width = baseInfo->width;

    if (baseInfo->height != 0)
        outInfo.height = baseInfo->height;

    outInfo.depth = baseInfo->depth;

    if (baseInfo->format != rhi::Format::UNDEFINED)
        outInfo.format = baseInfo->format;

    outInfo.textureUsage |= baseInfo->textureUsage;
    outInfo.samplesCount = baseInfo->samplesCount;
    outInfo.flags = baseInfo->flags;
}

}