#pragma once

#include "resource.h"
#include "common.h"

#include <variant>
#include <unordered_map>
#include <functional>

namespace fe::renderer
{

using ResourceInfoVariant = std::variant<rhi::BufferInfo, rhi::TextureInfo>;

class ResourceLayoutTracker;

// Must be used to create resources that are used as input and output by render passes in render graph. 
// In most cases those resources are color/depth targets and storage textures.
// Resources like vertex, index buffers, model textures, etc. must be allocated by another objects
class RenderGraphResourceManager
{
public:
    using SchedulingInfoConfigurator = std::function<void(ResourceSchedulingInfo&)>;

    RenderGraphResourceManager(ResourceLayoutTracker* resourceLayoutTracker);

    void begin_frame();
    void end_frame();

    uint32 get_rtv_descriptor(RenderPassName renderPassName, ResourceName textureName, uint32 mipLevel = 0) const;
    uint32 get_dsv_desciptor(RenderPassName renderPassName, ResourceName textureName) const;
    uint32 get_texture_uav_descriptor(RenderPassName renderPassName, ResourceName textureName, uint32 mipLevel = 0) const;
    uint32 get_texture_srv_descriptor(RenderPassName renderPassName, ResourceName textureName, uint32 mipLevel = 0) const;
    uint32 get_sampler_descriptor(ResourceName samplerName) const;

    Resource* get_resource(ResourceName resourceName);
    const Resource* get_resource(ResourceName resourceName) const;

    void begin_resource_scheduling();
    void end_resource_scheduling();
    void allocate_scheduled_resources();

    void queue_resource_allocation(
        RenderPassName renderPassName,
        ResourceName resourceName,
        const ResourceInfoVariant& info,
        const SchedulingInfoConfigurator& configurator
    );

    void queue_resource_usage(RenderPassName renderPassName, ResourceName resourceName, const SchedulingInfoConfigurator& configurator);
    void create_sampler(ResourceName samplerName, const rhi::SamplerInfo& samplerInfo);

private:
    using ResourceMap = std::unordered_map<Name, uint64>;
    using ResourceList = std::vector<Resource>;
    using SamplerMap = std::unordered_map<ResourceName, rhi::Sampler*>;
    using IntersectionEntryList = std::vector<Resource::IntersectionEntry>;

    struct SchedulingRequest
    {
        SchedulingInfoConfigurator configurator;
        RenderPassName renderPassName;
        ResourceName resourceName;
    };

    struct ResourceCreationRequest
    {
        ResourceInfoVariant info;
        RenderPassName renderPassName;
        ResourceName resourceName;
    };

    ResourceLayoutTracker* m_resourceLayoutTracker;

    std::vector<SchedulingRequest> m_schedulingAllocationRequests;
    std::vector<SchedulingRequest> m_schedulingUsageRequests;
    std::vector<ResourceCreationRequest> m_primaryResourceCreationRequests;

    ResourceMap m_previousFrameResourceMap;
    ResourceMap m_currentFrameResourceMap;
    ResourceList m_previousFrameResourceList;
    ResourceList m_currentFrameResourceList;
    IntersectionEntryList m_previousFrameIntersectionEntryList;
    IntersectionEntryList m_currentFrameIntersectionEntryList;

    SamplerMap m_samplers;

    void create_resource(const ResourceCreationRequest& request);
    bool transfer_previous_frame_resources();

    const Texture& get_texture_internal(
        RenderPassName renderPassName,
        ResourceName textureName,
        uint32 mipLevel,
        rhi::ResourceUsage mustHaveUsage,
        rhi::ResourceLayout mustHaveLayout
    ) const;
};

}