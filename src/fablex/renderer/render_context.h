#pragma once

#include "common.h"
#include "render_surface.h"

namespace fe::renderer
{

class RenderGraphResourceManager;
class RenderGraph;
class PipelineManager;
class PushConstantsManager;
class CommandManager;
class SceneManager;
class SynchronizationManager;
class ShaderManager;

struct RenderContextInfo
{
    RenderSurface renderSurface;
    RenderGraphResourceManager* renderGraphResourceManager = nullptr;
    PipelineManager* pipelineManager = nullptr;
    CommandManager* commandManager = nullptr;
    SceneManager* sceneManager = nullptr;
    SynchronizationManager* syncManager = nullptr;
    ShaderManager* shaderManager = nullptr;
    RenderGraph* renderGraph = nullptr;
};

class RenderContext
{
public:
    RenderContext(const RenderContextInfo& info) : m_info(info) 
    {
        FE_CHECK(m_info.renderGraphResourceManager);
        FE_CHECK(m_info.pipelineManager);
        FE_CHECK(m_info.commandManager);
        FE_CHECK(m_info.sceneManager);
        FE_CHECK(m_info.syncManager);
        FE_CHECK(m_info.shaderManager);
        FE_CHECK(m_info.renderGraph);
    }

    const RenderSurface& render_surface() const { return m_info.renderSurface; }
    RenderGraphResourceManager* render_graph_resource_manager() const { return m_info.renderGraphResourceManager; }
    PipelineManager* pipeline_manager() const { return m_info.pipelineManager; }
    CommandManager* command_manager() const { return m_info.commandManager; }
    SceneManager* scene_manager() const { return m_info.sceneManager; }
    SynchronizationManager* sync_manager() const { return m_info.syncManager; }
    ShaderManager* shader_manager() const { return m_info.shaderManager; }
    RenderGraph* render_graph() const { return m_info.renderGraph; }

    void set_main_swap_chain(rhi::SwapChain* mainSwapChain) { m_info.renderSurface.mainSwapChain = mainSwapChain; }

private:
    RenderContextInfo m_info;
};

}