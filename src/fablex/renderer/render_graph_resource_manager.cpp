#include "render_graph_resource_manager.h"
#include "resource_layout_tracker.h"
#include "rhi/rhi.h"
#include "core/logger.h"
#include "core/utils.h"
#include "rhi/json_serialization.h"

#include <algorithm>

namespace fe::renderer
{

RenderGraphResourceManager::RenderGraphResourceManager(ResourceLayoutTracker* resourceLayoutTracker)
    : m_resourceLayoutTracker(resourceLayoutTracker)
{
    FE_CHECK(m_resourceLayoutTracker);
}

RenderGraphResourceManager::~RenderGraphResourceManager()
{
    m_currentFrameResourceList.clear();
    m_previousFrameResourceList.clear();
}

template <typename T>
void move_swap(T& a, T& b) {
    T temp = std::move(a);
    a = std::move(b);
    b = std::move(temp);
}

void RenderGraphResourceManager::begin_frame()
{
    m_previousFrameResourceList.clear();
    m_previousFrameResourceMap.clear();
    m_previousFrameIntersectionEntryList.clear();
    
    move_swap(m_previousFrameResourceList, m_currentFrameResourceList);
    move_swap(m_previousFrameResourceMap, m_currentFrameResourceMap);
    move_swap(m_previousFrameIntersectionEntryList, m_currentFrameIntersectionEntryList);
}

void RenderGraphResourceManager::end_frame()
{

}

uint32 RenderGraphResourceManager::get_rtv_descriptor(
    RenderPassName renderPassName, 
    ResourceName textureName, 
    uint32 mipLevel
) const
{
    return get_texture_internal(
        renderPassName, 
        textureName, 
        mipLevel, 
        rhi::ResourceUsage::COLOR_ATTACHMENT, 
        rhi::ResourceLayout::COLOR_ATTACHMENT
    ).get_rtv_descriptor();
}

uint32 RenderGraphResourceManager::get_dsv_desciptor(RenderPassName renderPassName, ResourceName textureName) const
{
    return get_texture_internal(
        renderPassName, 
        textureName, 
        0, 
        rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT, 
        rhi::ResourceLayout::DEPTH_STENCIL
    ).get_dsv_descriptor();
}

uint32 RenderGraphResourceManager::get_texture_uav_descriptor(
    RenderPassName renderPassName, 
    ResourceName textureName, 
    uint32 mipLevel
) const
{
    return get_texture_internal(
        renderPassName, 
        textureName, 
        mipLevel, 
        rhi::ResourceUsage::STORAGE_TEXTURE, 
        rhi::ResourceLayout::GENERAL
    ).get_uav_descriptor();
}

uint32 RenderGraphResourceManager::get_texture_srv_descriptor(
    RenderPassName renderPassName, 
    ResourceName textureName, 
    uint32 mipLevel
) const
{
    return get_texture_internal(
        renderPassName, 
        textureName, 
        mipLevel, 
        rhi::ResourceUsage::SAMPLED_TEXTURE, 
        rhi::ResourceLayout::SHADER_READ
    ).get_srv_descriptor();
}

Resource* RenderGraphResourceManager::get_resource(ResourceName resourceName)
{
    FE_CHECK(resourceName.is_valid());

    auto it = m_currentFrameResourceMap.find(resourceName);
    if (it == m_currentFrameResourceMap.end())
        return nullptr;

    return &m_currentFrameResourceList.at(it->second);
}

const Resource* RenderGraphResourceManager::get_resource(ResourceName resourceName) const
{
    auto it = m_currentFrameResourceMap.find(resourceName);
    if (it == m_currentFrameResourceMap.end())
        return nullptr;

    return &m_currentFrameResourceList.at(it->second);
}

void RenderGraphResourceManager::begin_resource_scheduling()
{
    m_schedulingAllocationRequests.clear();
    m_schedulingUsageRequests.clear();
    m_primaryResourceCreationRequests.clear();
}

void RenderGraphResourceManager::end_resource_scheduling()
{
    for (const ResourceCreationRequest& request : m_primaryResourceCreationRequests)
    {
        FE_CHECK(!get_resource(request.resourceName));
        create_resource(request);
    }

    for (const SchedulingRequest& request : m_schedulingAllocationRequests)
    {
        Resource* resource = get_resource(request.resourceName);
        if (!resource)
            FE_LOG(LogRenderer, FATAL, "Tries to use resource {} that was not created.", request.resourceName);

        request.configurator(resource->get_scheduling_info());
    }

    for (const SchedulingRequest& request : m_schedulingUsageRequests)
    {
        Resource* resource = get_resource(request.resourceName);
        if (!resource)
            FE_LOG(LogRenderer, FATAL, "Tries to use resource {} that was not created.", request.resourceName);

        request.configurator(resource->get_scheduling_info());
    }
}

void RenderGraphResourceManager::allocate_scheduled_resources()
{
    if (transfer_previous_frame_resources())
        return;

    for (Resource& resource : m_currentFrameResourceList)
    {
        if (resource.is_valid()) continue;

        auto resIdxIt = m_currentFrameResourceMap.find(resource.get_name());
        const ResourceCreationRequest* creationRequest = nullptr;
        creationRequest = &m_primaryResourceCreationRequests.at(resIdxIt->second);

        std::visit(Utils::make_visitor(
            [this, &resource](const rhi::TextureInfo& textureInfo)
            {
                rhi::TextureHandle textureHandle;
                rhi::create_texture(&textureHandle, &textureInfo);
                resource.set_texture(textureHandle);
                m_resourceLayoutTracker->begin_resource_tracking(&resource);
            },
            [this, &resource](const rhi::BufferInfo& bufferInfo)
            {
                rhi::BufferHandle bufferHandle;
                rhi::create_buffer(&bufferHandle, &bufferInfo);
                resource.set_buffer(bufferHandle);
                m_resourceLayoutTracker->begin_resource_tracking(&resource);
            }
        ), creationRequest->info);
    }
}

void RenderGraphResourceManager::queue_resource_allocation(
    RenderPassName renderPassName,
    ResourceName resourceName,
    const ResourceInfoVariant& info,
    const SchedulingInfoConfigurator& configurator
)
{
    m_schedulingAllocationRequests.emplace_back(SchedulingRequest(configurator, renderPassName, resourceName));
    m_primaryResourceCreationRequests.emplace_back(ResourceCreationRequest(info, renderPassName, resourceName));
}

void RenderGraphResourceManager::queue_resource_usage(RenderPassName renderPassName, ResourceName resourceName, const SchedulingInfoConfigurator& configurator)
{
    m_schedulingUsageRequests.emplace_back(SchedulingRequest(configurator, renderPassName, resourceName));
}

void RenderGraphResourceManager::create_resource(const ResourceCreationRequest& request)
{
    std::visit(Utils::make_visitor(
        [&request, this](const rhi::TextureInfo& textureInfo)
        {
            m_currentFrameResourceList.emplace_back(request.resourceName, textureInfo.mipLevels);
            m_currentFrameResourceMap[request.resourceName] = m_currentFrameResourceList.size() - 1;
        },
        [&request, this](const rhi::BufferInfo& bufferInfo)
        {
            m_currentFrameResourceList.emplace_back(request.resourceName, 1);
            m_currentFrameResourceMap[request.resourceName] = m_currentFrameResourceList.size() - 1;
        }
    ), request.info);
}

bool RenderGraphResourceManager::transfer_previous_frame_resources()
{
    for (const Resource& resource : m_currentFrameResourceList)
    {
        m_currentFrameIntersectionEntryList.emplace_back(resource.get_name());
    }

    std::sort(m_currentFrameIntersectionEntryList.begin(), m_currentFrameIntersectionEntryList.end(), 
        [](Resource::IntersectionEntry& first, Resource::IntersectionEntry& second)
        {
            return first.name.to_id() < second.name.to_id();
        });

    std::vector<Resource::IntersectionEntry> intersectionResult;
    std::ranges::set_intersection(
        m_previousFrameIntersectionEntryList,
        m_currentFrameIntersectionEntryList,
        std::back_inserter(intersectionResult),
        [](Resource::IntersectionEntry& first, Resource::IntersectionEntry& second)
        {
            return first < second;
        }
    );

    for (const Resource::IntersectionEntry& entry : intersectionResult)
    {
        auto resIdxInPrevFrameIt = m_previousFrameResourceMap.find(entry.name);
        auto resIdxInCurrFrameIt = m_currentFrameResourceMap.find(entry.name);

        Resource& prevResource = m_previousFrameResourceList.at(resIdxInPrevFrameIt->second);
        Resource& currResource = m_currentFrameResourceList.at(resIdxInCurrFrameIt->second);

        currResource.set_from_resource(prevResource);
    }

    bool isMemoryLayoutValid = m_previousFrameIntersectionEntryList.size() == m_currentFrameIntersectionEntryList.size()
        && intersectionResult.size() == m_currentFrameIntersectionEntryList.size();

    return isMemoryLayoutValid;
}

const Texture& RenderGraphResourceManager::get_texture_internal(
    RenderPassName renderPassName,
    ResourceName textureName,
    uint32 mipLevel,
    rhi::ResourceUsage mustHaveUsage,
    rhi::ResourceLayout mustHaveLayout
) const
{
    const Resource* resource = get_resource(textureName);
    if (!resource->is_texture())
        FE_LOG(LogRenderer, FATAL, "Resource {} is not a valid texture.", textureName.to_string());

    const ResourceSchedulingInfo::RenderPassInfo* passInfo = resource->get_scheduling_info().get_render_pass_info(renderPassName);
    if (!passInfo)
        FE_LOG(LogRenderer, FATAL, "Resource {} is not scheduled for render pass {}.", textureName.to_string(), renderPassName.to_string());

    const Texture& texture = resource->get_texture();

    if (!has_flag(texture.get_handle()->textureUsage, mustHaveUsage))
        FE_LOG(LogRenderer, FATAL, "Texture {} does not have usage {}", textureName, to_string(mustHaveUsage));

    const std::optional<ResourceSchedulingInfo::ViewInfo>& viewInfo = passInfo->viewInfos[mipLevel];
    if (viewInfo == std::nullopt)
        FE_LOG(LogRenderer, FATAL, "Texture {} does not have view for mip level {}", textureName, mipLevel);

    if (!has_flag(viewInfo->requestedLayout, mustHaveLayout))
        FE_LOG(LogRenderer, FATAL, "Texture {} requested layout is not {} for render pass {}", textureName, to_string(mustHaveLayout), renderPassName);
    
    return texture;
}
    
}