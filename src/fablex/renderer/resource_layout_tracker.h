#pragma once

#include "common.h"
#include "rhi/resources.h"
#include <optional>
#include <unordered_map>

namespace fe::renderer
{

class Resource;

class ResourceLayoutTracker
{
public:
    struct ViewLayout
    {
        uint32 viewIndex = 0;
        rhi::ResourceLayout layout = rhi::ResourceLayout::UNDEFINED;
    };

    using ViewLayoutArray = std::vector<ViewLayout>;

    void begin_frame();
    void end_frame();

    void begin_resource_tracking(const Resource* resource);

    std::optional<rhi::PipelineBarrier> get_transition_to_layout(
        const Resource* resource,
        rhi::ResourceLayout newLayout,
        uint32 viewIndex
    );

private:
    std::unordered_map<ResourceName, ViewLayoutArray> m_viewLayoutsByName;

    ViewLayoutArray& get_view_layouts(const Resource* resource);
    bool is_new_layout_redundant(rhi::ResourceLayout currentLayout, rhi::ResourceLayout newLayout);
};

}