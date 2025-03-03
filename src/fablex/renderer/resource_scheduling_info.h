#pragma once

#include "common.h"
#include "rhi/resources.h"

#include <vector>
#include <optional>
#include <unordered_map>

namespace fe::renderer
{

class ResourceSchedulingInfo
{
public:
    struct ViewInfo
    {
        rhi::ResourceLayout requestedLayout = rhi::ResourceLayout::UNDEFINED;
    };

    struct RenderPassInfo
    {
        std::vector<std::optional<ViewInfo>> viewInfos;
    };

    ResourceSchedulingInfo(ResourceName resourceName, uint64 viewCount);

    void add_view_info(RenderPassName renderPassName, uint64 viewIndex, rhi::ResourceLayout layout);

    ResourceName get_name() const { return m_name; }
    uint64 get_view_count() const { return m_viewCount; }
    RenderPassInfo* get_render_pass_info(RenderPassName renderPassName);
    const RenderPassInfo* get_render_pass_info(RenderPassName renderPassName) const;

private:
    ResourceName m_name;
    uint64 m_viewCount = 0;
    std::unordered_map<RenderPassName, RenderPassInfo> m_renderPassInfoMap;
};

}