#pragma once

#include "common.h"
#include "render_pass.h"
#include <unordered_map>
#include <unordered_set>

namespace fe::renderer
{

class RenderGraph
{
public:
    using ViewName = uint64;

    class Node
    {
    public:
        using ViewNameSet = std::unordered_set<ViewName>;

        void add_read_dependency(ResourceName resourceName, uint32 viewCount);
        void add_read_dependency(ResourceName resourceName, uint32 firstViewIndex, uint32 lastViewIndex);

        void add_write_dependency(ResourceName resourceName, uint32 viewCount);
        void add_write_dependency(ResourceName resourceName, uint32 firstViewIndex, uint32 lastViewIndex);

        const RenderPassInfo& get_info() const { return m_renderPassMetadata; }
        const ViewNameSet& get_read_views() const { return m_readViews; }
        const ViewNameSet& get_write_views() const { return m_writeViews; }
        const ViewNameSet& get_all_views() const { return m_allViews; }

    private:
        RenderPassInfo m_renderPassMetadata;

        ViewNameSet m_readViews;
        ViewNameSet m_writeViews;
        ViewNameSet m_allViews; 
    };

    Node* get_node(RenderPassName renderPassName) const;

private:
    using NodeMap = std::unordered_map<RenderPassName, Node*>;

    NodeMap m_nodeByName;
};

}