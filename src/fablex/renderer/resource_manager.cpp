#include "resource_manager.h"
#include "rhi/rhi.h"
#include "core/logger.h"
#include "core/utils.h"

#include <algorithm>

namespace fe::renderer
{

void ResourceManager::begin_frame()
{
    m_previousFrameResourceList.clear();
    m_previousFrameResourceMap.clear();
    m_previousFrameIntersectionEntryList.clear();
    
    std::swap(m_previousFrameResourceList, m_currentFrameResourceList);
    std::swap(m_previousFrameResourceMap, m_currentFrameResourceMap);
    std::swap(m_previousFrameIntersectionEntryList, m_currentFrameIntersectionEntryList);
}

void ResourceManager::end_frame()
{

}

uint32 ResourceManager::get_render_target_descriptor(RenderPassName renderPassName, ResourceName resourceName, uint32 mipLevel) const
{
    const Resource* resource = get_resource(resourceName);
    if (!resource->is_texture())
        FE_LOG(LogRenderer, FATAL, "Resource {} is not a texture.", resourceName.to_string());

    const ResourceSchedulingInfo::RenderPassInfo* passInfo = resource->get_scheduling_info().get_render_pass_info(renderPassName);
    if (!passInfo)
        FE_LOG(LogRenderer, FATAL, "Resource {} is not scheduled for render pass {}.", resourceName.to_string(), renderPassName.to_string());

    const Texture& texture = resource->get_texture();
    FE_CHECK(has_flag(texture.get_handle()->textureUsage, rhi::ResourceUsage::COLOR_ATTACHMENT));

    const std::optional<ResourceSchedulingInfo::ViewInfo>& viewInfo = passInfo->viewInfos[mipLevel];
    if (viewInfo == std::nullopt)
        FE_LOG(LogRenderer, FATAL, "Resource {} does not have view for mip level {}", resourceName.to_string(), mipLevel);

    return resource->get_texture().get_rtv_descriptor();
}

uint32 ResourceManager::get_depth_stencil_desciptor(RenderPassName renderPassName, ResourceName resourceName) const
{
    const Resource* resource = get_resource(resourceName);
    if (!resource->is_texture())
        FE_LOG(LogRenderer, FATAL, "Resource {} is not a texture.", resourceName.to_string());

    const ResourceSchedulingInfo::RenderPassInfo* passInfo = resource->get_scheduling_info().get_render_pass_info(renderPassName);
    if (!passInfo)
        FE_LOG(LogRenderer, FATAL, "Resource {} is not scheduled for render pass {}.", resourceName.to_string(), renderPassName.to_string());

    const Texture& texture = resource->get_texture();
    FE_CHECK(has_flag(texture.get_handle()->textureUsage, rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT));

    return texture.get_dsv_descriptor();
}

uint32 ResourceManager::get_sampler_descriptor(ResourceName samplerName) const
{
    auto it = m_samplers.find(samplerName);

    if (it == m_samplers.end())
        FE_LOG(LogRenderer, FATAL, "No sampler with name {}", samplerName.to_string());

    return it->second->descriptorIndex;
}

Resource* ResourceManager::get_resource(ResourceName resourceName)
{
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

void ResourceManager::allocate_scheduled_resources()
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
            [&resource](const rhi::TextureInfo& textureInfo)
            {
                rhi::TextureHandle textureHandle;
                rhi::create_texture(&textureHandle, &textureInfo);
                resource.set_texture(Texture(textureHandle));
                rhi::set_name(textureHandle, resource.get_name().to_string());
            },
            [&resource](const rhi::BufferInfo& bufferInfo)
            {
                rhi::BufferHandle bufferHandle;
                rhi::create_buffer(&bufferHandle, &bufferInfo);
                resource.set_buffer(Buffer(bufferHandle));
                rhi::set_name(bufferHandle, resource.get_name().to_string());
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

void ResourceManager::create_sampler(ResourceName samplerName, const rhi::SamplerInfo& samplerInfo)
{
    if (m_samplers.contains(samplerName))
    {
        FE_LOG(LogRenderer, ERROR, "Sampler {} already exists.", samplerName.to_string());
        return;
    }

    rhi::Sampler* sampler = nullptr;
    rhi::create_sampler(&sampler, &samplerInfo);
    m_samplers[samplerName] = sampler;
}

void ResourceManager::create_resource(const ResourceCreationRequest& request)
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

bool ResourceManager::transfer_previous_frame_resources()
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
            return first == second;
        }
    );

    for (const Resource::IntersectionEntry& entry : intersectionResult)
    {
        auto resIdxInPrevFrameIt = m_previousFrameResourceMap.find(entry.name);
        auto resIdxInCurrFrameIt = m_currentFrameResourceMap.find(entry.name);

        Resource& prevResource = m_previousFrameResourceList.at(resIdxInPrevFrameIt->second);
        Resource& currResource = m_currentFrameResourceList.at(resIdxInCurrFrameIt->second);

        if (prevResource.is_buffer())
            currResource.set_buffer(std::move(prevResource.get_buffer()));

        if (prevResource.is_texture())
            currResource.set_texture(std::move(prevResource.get_texture()));
    }

    bool isMemoryLayoutValid = m_previousFrameIntersectionEntryList.size() == m_currentFrameIntersectionEntryList.size()
        && intersectionResult.size() == m_currentFrameIntersectionEntryList.size();

    return isMemoryLayoutValid;
}
    
}