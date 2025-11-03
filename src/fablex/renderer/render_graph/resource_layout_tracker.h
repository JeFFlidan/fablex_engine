#pragma once

#include "common.h"
#include "fwd.h"
#include "rhi_types.h"
#include "rhi/resources/pipeline_barrier.h"
#include <optional>
#include <unordered_map>

namespace fe::renderer::rg
{

class ResourceLayoutTracker
{
public:
    struct ViewLayout
    {
        uint32 viewIndex = 0;
        ResourceLayout layout = ResourceLayout::UNDEFINED;
    };

    using ViewLayoutArray = std::vector<ViewLayout>;

    void begin_frame();
    void end_frame();

    void begin_resource_tracking(const Resource* resource);

    std::optional<PipelineBarrier> get_transition_to_layout(
        const Resource* resource,
        ResourceLayout newLayout,
        uint32 viewIndex
    );

private:
    std::unordered_map<ResourceName, ViewLayoutArray> m_viewLayoutsByName;

    ViewLayoutArray& get_view_layouts(const Resource* resource);
    bool is_new_layout_redundant(ResourceLayout currentLayout, ResourceLayout newLayout);
};

}