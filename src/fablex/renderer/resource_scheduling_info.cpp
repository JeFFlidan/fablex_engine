#include "resource_scheduling_info.h"
#include "core/macro.h"

namespace fe::renderer
{

ResourceSchedulingInfo::ResourceSchedulingInfo(ResourceName resourceName, uint64 viewCount)
    : m_name(resourceName), m_viewCount(viewCount)
{
    
}

void ResourceSchedulingInfo::add_view_info(RenderPassName renderPassName, uint64 viewIndex, rhi::ResourceLayout layout)
{
    FE_CHECK_MSG(viewIndex < m_viewCount, "View index is out of bounds.");

    RenderPassInfo& passInfo = m_renderPassInfoMap[renderPassName];
    passInfo.viewInfos.resize(m_viewCount);
    passInfo.viewInfos[viewIndex] = ViewInfo();
    passInfo.viewInfos[viewIndex]->requestedLayout = layout;
}

ResourceSchedulingInfo::RenderPassInfo* ResourceSchedulingInfo::get_render_pass_info(RenderPassName renderPassName)
{
    auto it = m_renderPassInfoMap.find(renderPassName);
    return it != m_renderPassInfoMap.end() ? &it->second : nullptr;
}

const ResourceSchedulingInfo::RenderPassInfo* ResourceSchedulingInfo::get_render_pass_info(RenderPassName renderPassName) const
{
    auto it = m_renderPassInfoMap.find(renderPassName);
    return it != m_renderPassInfoMap.end() ? &it->second : nullptr;
}

}