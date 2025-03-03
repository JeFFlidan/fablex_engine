#pragma once

#include "common.h"

namespace fe::renderer
{

class ResourceManager;
class RenderGraph;
class PipelineManager;

struct RenderContextInfo
{
    RenderSurface renderSurface;
    ResourceManager* resourceManager;
    PipelineManager* pipelineManager;
    RenderGraph* renderGraph;
};

class RenderContext
{
public:
    RenderContext(const RenderContextInfo& info) : m_info(info) 
    {
        FE_CHECK(m_info.resourceManager);
        FE_CHECK(m_info.pipelineManager);
        FE_CHECK(m_info.renderGraph);
    }

    const RenderSurface& get_render_surface() const { return m_info.renderSurface; }
    ResourceManager* get_resource_manager() const { return m_info.resourceManager; }
    RenderGraph* get_render_graph() const { return m_info.renderGraph; }

private:
    RenderContextInfo m_info;
};

}