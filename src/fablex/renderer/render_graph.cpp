#include "render_graph.h"

namespace fe::renderer
{

RenderGraph::Node* RenderGraph::get_node(RenderPassName renderPassName) const
{
    auto it = m_nodeByName.find(renderPassName);
    if (it == m_nodeByName.end())
        return nullptr;

    return it->second;
}

void RenderGraph::Node::add_read_dependency(ResourceName resourceName, uint32 viewCount)
{

}

void RenderGraph::Node::add_read_dependency(ResourceName resourceName, uint32 firstViewIndex, uint32 lastViewIndex)
{

}

void RenderGraph::Node::add_write_dependency(ResourceName resourceName, uint32 viewCount)
{

}

void RenderGraph::Node::add_write_dependency(ResourceName resourceName, uint32 firstViewIndex, uint32 lastViewIndex)
{

}

}