#pragma once

#include "resource.h"
#include "common.h"

#include <variant>
#include <unordered_map>
#include <functional>

namespace fe::renderer
{

using ResourceInfoVariant = std::variant<rhi::BufferInfo, rhi::TextureInfo>;

class ResourceManager
{
public:
    using SchedulingInfoConfigurator = std::function<void(ResourceSchedulingInfo&)>;

    void begin_frame();
    void end_frame();

    uint32 get_render_target_descriptor(RenderPassName renderPassName, ResourceName resourceName, uint32 mipLevel = 0) const;
    uint32 get_depth_stencil_desciptor(RenderPassName renderPassName, ResourceName resourceName) const;
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
};

}