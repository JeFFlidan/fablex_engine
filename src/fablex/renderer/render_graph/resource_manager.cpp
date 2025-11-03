#include "resource_manager.h"
#include "resource_layout_tracker.h"
#include "rhi/rhi.h"
#include "core/logger.h"
#include "core/utils.h"
#include "rhi/json_serialization.h"

#include <algorithm>

namespace fe::renderer::rg
{

ResourceManager::ResourceManager(ResourceLayoutTracker* resourceLayoutTracker)
    : m_resourceLayoutTracker(resourceLayoutTracker)
{
    FE_CHECK(m_resourceLayoutTracker);
}

ResourceManager::~ResourceManager()
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

void ResourceManager::begin_frame()
{
    m_previousFrameResourceList.clear();
    m_previousFrameResourceMap.clear();
    m_previousFrameIntersectionEntryList.clear();
    
    move_swap(m_previousFrameResourceList, m_currentFrameResourceList);
    move_swap(m_previousFrameResourceMap, m_currentFrameResourceMap);
    move_swap(m_previousFrameIntersectionEntryList, m_currentFrameIntersectionEntryList);
}

void ResourceManager::end_frame()
{

}

uint32 ResourceManager::rtv_descriptor(
    RenderPassName renderPassName, 
    ResourceName textureName, 
    uint32 mipLevel
) const
{
    return get_texture_internal(
        renderPassName, 
        textureName, 
        mipLevel, 
        ResourceUsage::COLOR_ATTACHMENT, 
        ResourceLayout::COLOR_ATTACHMENT
    ).rtv_descriptor();
}

uint32 ResourceManager::dsv_desciptor(RenderPassName renderPassName, ResourceName textureName) const
{
    return get_texture_internal(
        renderPassName, 
        textureName, 
        0, 
        ResourceUsage::DEPTH_STENCIL_ATTACHMENT, 
        ResourceLayout::DEPTH_STENCIL
    ).dsv_descriptor();
}

uint32 ResourceManager::texture_uav_descriptor(
    RenderPassName renderPassName, 
    ResourceName textureName, 
    uint32 mipLevel
) const
{
    return get_texture_internal(
        renderPassName, 
        textureName, 
        mipLevel, 
        ResourceUsage::STORAGE_TEXTURE, 
        ResourceLayout::GENERAL
    ).uav_descriptor();
}

uint32 ResourceManager::texture_srv_descriptor(
    RenderPassName renderPassName, 
    ResourceName textureName, 
    uint32 mipLevel
) const
{
    return get_texture_internal(
        renderPassName, 
        textureName, 
        mipLevel, 
        ResourceUsage::SAMPLED_TEXTURE, 
        ResourceLayout::SHADER_READ
    ).srv_descriptor();
}

Resource* ResourceManager::get_resource(ResourceName resourceName)
{
    FE_CHECK(resourceName.is_valid());

    auto it = m_currentFrameResourceMap.find(resourceName);
    if (it == m_currentFrameResourceMap.end())
        return nullptr;

    return &m_currentFrameResourceList.at(it->second);
}

const Resource* ResourceManager::get_resource(ResourceName resourceName) const
{
    auto it = m_currentFrameResourceMap.find(resourceName);
    if (it == m_currentFrameResourceMap.end())
        return nullptr;

    return &m_currentFrameResourceList.at(it->second);
}

void ResourceManager::begin_resource_scheduling()
{
    m_schedulingAllocationRequests.clear();
    m_schedulingUsageRequests.clear();
    m_primaryResourceCreationRequests.clear();
}

void ResourceManager::end_resource_scheduling()
{
    for (const ResourceCreationRequest& request : m_primaryResourceCreationRequests)
    {
        // SVGF needs outputs from the previous render graph execution and assertion is fired because of this, so I decided to use WARNING log.
        if (get_resource(request.resourceName))
        {
            FE_LOG(LogRenderer, WARNING, "Resource {} allocation is already requested by {}", request.resourceName, request.renderPassName);
            continue;
        }

        create_resource(request);
    }
    
    for (const SchedulingRequest& request : m_schedulingAllocationRequests)
    {
        Resource* resource = get_resource(request.resourceName);
        if (!resource)
            FE_LOG(LogRenderer, FATAL, "Tries to use resource {} that was not created.", request.resourceName);

        request.configurator(resource->scheduling_info());
    }

    for (const SchedulingRequest& request : m_schedulingUsageRequests)
    {
        Resource* resource = get_resource(request.resourceName);
        if (!resource)
            FE_LOG(LogRenderer, FATAL, "Tries to use resource {} that was not created.", request.resourceName);
        
        request.configurator(resource->scheduling_info());
    }
}

void ResourceManager::allocate_scheduled_resources()
{
    if (transfer_previous_frame_resources())
        return;

    for (Resource& resource : m_currentFrameResourceList)
    {
        if (resource.is_valid()) continue;

        auto resIdxIt = m_currentFrameResourceMap.find(resource.name());
        const ResourceCreationRequest* creationRequest = nullptr;
        creationRequest = &m_primaryResourceCreationRequests.at(resIdxIt->second);

        std::visit(Utils::make_visitor(
            [this, &resource](const TextureCreateInfo& textureInfo)
            {
                resource.set_texture(textureInfo);
                m_resourceLayoutTracker->begin_resource_tracking(&resource);
            },
            [this, &resource](const BufferCreateInfo& bufferInfo)
            {
                resource.set_buffer(bufferInfo);
                m_resourceLayoutTracker->begin_resource_tracking(&resource);
            }
        ), creationRequest->info);
    }
}

void ResourceManager::queue_resource_allocation(
    RenderPassName renderPassName,
    ResourceName resourceName,
    const ResourceInfoVariant& info,
    const SchedulingInfoConfigurator& configurator
)
{
    m_schedulingAllocationRequests.emplace_back(SchedulingRequest(configurator, renderPassName, resourceName));
    m_primaryResourceCreationRequests.emplace_back(ResourceCreationRequest(info, renderPassName, resourceName));
}

void ResourceManager::queue_resource_usage(RenderPassName renderPassName, ResourceName resourceName, const SchedulingInfoConfigurator& configurator)
{
    m_schedulingUsageRequests.emplace_back(SchedulingRequest(configurator, renderPassName, resourceName));
}

void ResourceManager::create_resource(const ResourceCreationRequest& request)
{
    std::visit(Utils::make_visitor(
        [&request, this](const TextureCreateInfo& textureInfo)
        {
            m_currentFrameResourceList.emplace_back(request.resourceName, textureInfo.mipLevels);
            m_currentFrameResourceMap[request.resourceName] = m_currentFrameResourceList.size() - 1;
        },
        [&request, this](const BufferCreateInfo& bufferInfo)
        {
            m_currentFrameResourceList.emplace_back(request.resourceName, 1);
            m_currentFrameResourceMap[request.resourceName] = m_currentFrameResourceList.size() - 1;
        }
    ), request.info);
}

bool ResourceManager::transfer_previous_frame_resources()
{
    for (const Resource& resource : m_currentFrameResourceList)
    {
        m_currentFrameIntersectionEntryList.emplace_back(resource.name());
    }

    std::sort(m_currentFrameIntersectionEntryList.begin(), m_currentFrameIntersectionEntryList.end(), 
        [](ResourceIntersectionEntry& first, ResourceIntersectionEntry& second)
        {
            return first.name.to_id() < second.name.to_id();
        });

    std::vector<ResourceIntersectionEntry> intersectionResult;
    std::ranges::set_intersection(
        m_previousFrameIntersectionEntryList,
        m_currentFrameIntersectionEntryList,
        std::back_inserter(intersectionResult),
        [](ResourceIntersectionEntry& first, ResourceIntersectionEntry& second)
        {
            return first < second;
        }
    );

    for (const ResourceIntersectionEntry& entry : intersectionResult)
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

const Texture& ResourceManager::get_texture_internal(
    RenderPassName renderPassName,
    ResourceName textureName,
    uint32 mipLevel,
    ResourceUsage mustHaveUsage,
    ResourceLayout mustHaveLayout
) const
{
    const Resource* resource = get_resource(textureName);
    if (!resource->is_texture())
        FE_LOG(LogRenderer, FATAL, "Resource {} is not a valid texture.", textureName.to_string());

    const ResourceSchedulingInfo::RenderPassInfo* passInfo = resource->scheduling_info().render_pass_info(renderPassName);
    if (!passInfo)
        FE_LOG(LogRenderer, FATAL, "Resource {} is not scheduled for render pass {}.", textureName.to_string(), renderPassName.to_string());

    const Texture& texture = resource->texture();

    if (!has_flag(texture.handle()->textureUsage, mustHaveUsage))
        FE_LOG(LogRenderer, FATAL, "Texture {} does not have usage {}", textureName, to_string(mustHaveUsage));

    const std::optional<ResourceSchedulingInfo::ViewInfo>& viewInfo = passInfo->viewInfos[mipLevel];
    if (viewInfo == std::nullopt)
        FE_LOG(LogRenderer, FATAL, "Texture {} does not have view for mip level {}", textureName, mipLevel);

    if (!has_flag(viewInfo->requestedLayout, mustHaveLayout))
    {
        FE_LOG(LogRenderer, FATAL, "Texture {} requested layout is not {} for render pass {}. Current layout is {}", 
            textureName, 
            to_string(mustHaveLayout), 
            renderPassName,
            to_string(viewInfo->requestedLayout)
        );
    }

    return texture;
}
    
}