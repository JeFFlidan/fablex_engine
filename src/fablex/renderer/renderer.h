#pragma once

#include "renderer_config.h"
#include "render_graph.h"
#include "render_pass_container.h"
#include "render_graph_resource_manager.h"
#include "pipeline_manager.h"
#include "command_manager.h"
#include "scene_manager/scene_manager.h"
#include "synchronization_manager.h"
#include "shader_manager.h"
#include "resource_layout_tracker.h"
#include "render_context.h"
#include "core/window.h"

namespace fe::renderer
{

struct RendererInfo
{
    const RendererConfig* config = nullptr;
    Window* window = nullptr;
};

class Renderer
{
public:
    Renderer(const RendererInfo& rendererInfo);
    ~Renderer();

    void draw();

private:
    struct DependencyLevelCommandContext
    {
        constexpr static uint32 s_undefinedDependencyLevel = ~0u;

        DependencyLevelCommandContext(uint32 dependencyLevelIdx) 
            : dependencyLevelIndex(dependencyLevelIdx) { }

        rhi::CommandBuffer* workerCmd = nullptr;
        uint32 dependencyLevelIndex = s_undefinedDependencyLevel;
        std::vector<const RenderGraph::Node*> nodesToRecord;
    };

    struct SubmitContext
    {
        rhi::Semaphore* signalSemaphore;
        std::vector<rhi::Semaphore*> waitSemaphores; 
        std::vector<DependencyLevelCommandContext> depencyLevelCommandContexts;
    };

    using SubmitContextArray = std::vector<SubmitContext>;
    using SubmitInfoArray = std::vector<rhi::SubmitInfo>;
    using PipelineBarrierArray = std::vector<rhi::PipelineBarrier>;

    std::unique_ptr<RenderGraph> m_renderGraph = nullptr;
    std::unique_ptr<RenderPassContainer> m_renderPassContainer = nullptr;
    std::unique_ptr<ResourceLayoutTracker> m_resourceLayoutTracker = nullptr;
    std::unique_ptr<RenderGraphResourceManager> m_resourceManager = nullptr;
    std::unique_ptr<PipelineManager> m_pipelineManager = nullptr;
    std::unique_ptr<CommandManager> m_commandManager = nullptr;
    std::unique_ptr<SceneManager> m_sceneManager = nullptr;
    std::unique_ptr<SynchronizationManager> m_syncManager = nullptr;
    std::unique_ptr<ShaderManager> m_shaderManager = nullptr;
    std::unique_ptr<RenderContext> m_renderContext = nullptr;

    Window* m_window = nullptr;
    const RendererConfig* m_config = nullptr;

    rhi::SwapChain* m_mainSwapChain = nullptr;
    rhi::Semaphore* m_acquireSemaphore = nullptr;
    rhi::Semaphore* m_uploadSemaphore = nullptr;
    rhi::Semaphore* m_bvhBuildSemaphore = nullptr;
    rhi::Semaphore* m_backBufferSemaphore = nullptr;
    
    rhi::SubmitInfo m_uploadSubmitInfo;
    rhi::SubmitInfo m_bvhBuildSubmitInfo;

    const RenderGraph::Node* m_backBufferNode = nullptr;

    std::vector<SubmitContextArray> m_submitContextsPerQueue;
    std::unordered_map<RenderPassName, PipelineBarrierArray> m_pipelineBarriersByPassName;

    TaskGroup m_commandRecordingTaskGroup;

    void init_rhi();
    void init_managers();
    void init_render_context();
    void create_main_swap_chain();
    void create_samplers();

    void acquire_next_image();
    void begin_frame();
    void end_frame();
    void schedule_frame();
    void prepare_render_graph_execution();
    void execute_render_graph();
    void present();
    
    void configure_submit_contexts();
    void configure_pipeline_barriers();

    void record_upload_cmd();
    void record_bvh_build_cmd();
    void record_worker_cmds();
    void submit();

    bool is_upload_cmd_submit_required() const;
    bool is_bvh_build_cmd_submit_required() const;
};

}