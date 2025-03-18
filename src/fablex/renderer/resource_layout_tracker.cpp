#include "resource_layout_tracker.h"
#include "resource.h"

namespace fe::renderer
{

void ResourceLayoutTracker::begin_frame()
{

}

void ResourceLayoutTracker::end_frame()
{

}

void ResourceLayoutTracker::begin_resource_tracking(const Resource* resource)
{
    auto it = m_viewLayoutsByName.find(resource->get_name());
    if (it != m_viewLayoutsByName.end())
        FE_LOG(LogRenderer, WARNING, "Resource {} has been already tracked.", resource->get_name());

    ViewLayoutArray& viewLayouts = m_viewLayoutsByName[resource->get_name()];
    viewLayouts.resize(resource->get_view_count());
    
    for (uint32 i = 0; i != resource->get_view_count(); ++i)
        viewLayouts[i].viewIndex = i;
}

std::optional<rhi::PipelineBarrier> ResourceLayoutTracker::get_transition_to_layout(
    const Resource* resource,
    rhi::ResourceLayout newLayout,
    uint32 viewIndex
)
{
    FE_CHECK(resource && resource->is_valid());

    ViewLayoutArray& viewStates = get_view_layouts(resource);
    
    if (viewIndex >= viewStates.size())
        FE_LOG(LogRenderer, FATAL, "Resource {} does not have view with index {}", resource->get_name(), viewIndex);
    
    rhi::ResourceLayout currentLayout = viewStates[viewIndex].layout;

    if (is_new_layout_redundant(currentLayout, newLayout))
        return std::nullopt;

    viewStates[viewIndex].layout = newLayout;

    if (resource->is_buffer())
        return rhi::PipelineBarrier(resource->get_buffer().get_handle(), currentLayout, newLayout);

    return rhi::PipelineBarrier(
        resource->get_texture().get_handle(),
        currentLayout,
        newLayout,
        viewIndex,
        1
    );
}

ResourceLayoutTracker::ViewLayoutArray& ResourceLayoutTracker::get_view_layouts(const Resource* resource)
{
    auto it = m_viewLayoutsByName.find(resource->get_name());
    FE_CHECK(it != m_viewLayoutsByName.end());
    return it->second;
}

bool ResourceLayoutTracker::is_new_layout_redundant(rhi::ResourceLayout currentLayout, rhi::ResourceLayout newLayout)
{
    return currentLayout == newLayout;
}

}