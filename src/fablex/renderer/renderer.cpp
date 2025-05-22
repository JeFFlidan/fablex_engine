#include "renderer.h"
#include "globals.h"
#include "utils.h"
#include "resource_scheduler.h"
#include "rhi/utils.h"
#include "core/task_composer.h"

namespace fe::renderer
{

Renderer::Renderer(const RendererInfo& rendererInfo)
    : m_window(rendererInfo.window), m_config(rendererInfo.config)
{
    FE_CHECK(m_window);
    FE_CHECK(m_config);

    init_rhi();
    init_managers();
    init_render_context();
    
    m_renderGraph->load_from_metadata(m_config->get_render_graph_metadata_path(), m_renderPassContainer.get());

    create_pipelines();
    
    create_main_swap_chain();
}

Renderer::~Renderer()
{
    for (uint32 queueIdx = 0; queueIdx != rhi::g_queueCount; ++queueIdx)
        rhi::wait_queue_idle((rhi::QueueType)queueIdx);

    rhi::destroy_swap_chain(m_mainSwapChain);
    m_pipelineManager.reset();
    m_shaderManager.reset();
    m_resourceManager.reset();
    m_syncManager.reset();
    m_commandManager.reset();
    m_sceneManager.reset();
    m_imGuiRenderer.reset();
    m_deletionQueue.reset();
    rhi::cleanup();
}

void Renderer::predraw()
{
    bool beginFrameCalled = false;

    rhi::CommandBuffer* cmd = nullptr;

    if (!m_imGuiRenderer->is_font_texture_created())
    {
        if (!beginFrameCalled)
        {
            m_commandManager->begin_frame();
            m_syncManager->begin_frame();
            beginFrameCalled = true;
        }

        if (!cmd)
        {
            cmd = m_commandManager->get_cmd(rhi::QueueType::GRAPHICS);
            rhi::begin_command_buffer(cmd);
        }

        m_imGuiRenderer->create_font_texture(cmd);

        if (!beginFrameCalled)
        {
            m_commandManager->begin_frame();
            m_syncManager->begin_frame();
        }
    }

    if (cmd)
    {
        rhi::end_command_buffer(cmd);

        rhi::SubmitInfo submitInfo;
        submitInfo.cmdBuffers.push_back(cmd);
        rhi::submit(&submitInfo, m_syncManager->get_fence());

        m_syncManager->wait_fences();
    }
}

void Renderer::draw()
{
    if (m_renderGraph->get_nodes().empty())
        return;

    ++g_frameNumber;

    acquire_next_image();
    begin_frame();
    schedule_frame();
    prepare_render_graph_execution();
    execute_render_graph();
    present();
    end_frame();
}

void Renderer::init_rhi()
{
    FE_LOG(LogRenderer, INFO, "Starting RHI initialization.");

    rhi::RHIInitInfo initInfo;
    initInfo.gpuPreference = rhi::GPUPreference::DISCRETE;
    initInfo.validationMode = m_config->get_validation_mode();

    rhi::fill_function_table(m_config->get_graphics_api());
    rhi::init(&initInfo);

    FE_LOG(LogRenderer, INFO, "RHI initialization completed.");
}

void Renderer::init_managers()
{
    FE_LOG(LogRenderer, INFO, "Starting renderer systems initialization");

    m_renderGraph = std::make_unique<RenderGraph>();
    m_resourceLayoutTracker = std::make_unique<ResourceLayoutTracker>();
    m_resourceManager = std::make_unique<RenderGraphResourceManager>(m_resourceLayoutTracker.get());
    m_commandManager = std::make_unique<CommandManager>();
    m_syncManager = std::make_unique<SynchronizationManager>();
    m_shaderManager = std::make_unique<ShaderManager>();
    m_pipelineManager = std::make_unique<PipelineManager>(m_shaderManager.get());
    m_sceneManager = std::make_unique<SceneManager>();
    m_deletionQueue = std::make_unique<DeletionQueue>();
    m_imGuiRenderer = std::make_unique<ImGuiRenderer>(m_deletionQueue.get(), m_shaderManager.get());

    FE_LOG(LogRenderer, INFO, "Renderer systems initialization completed.");
}

void Renderer::init_render_context()
{
    FE_LOG(LogRenderer, INFO, "Starting render context initialization.");

    RenderContextInfo info;
    info.renderGraph = m_renderGraph.get();
    info.commandManager = m_commandManager.get();
    info.renderGraphResourceManager = m_resourceManager.get();
    info.syncManager = m_syncManager.get();
    info.sceneManager = m_sceneManager.get();
    info.pipelineManager = m_pipelineManager.get();
    info.shaderManager = m_shaderManager.get();
    info.imGuiRenderer = m_imGuiRenderer.get();
    
    info.renderSurface.width = 1920;
    info.renderSurface.height = 1080;
    info.renderSurface.renderTargetFormat = rhi::Format::R8G8B8A8_UNORM;
    info.renderSurface.depthStencilFormat = rhi::Format::D32_SFLOAT;

    m_renderContext = std::make_unique<RenderContext>(info);
    m_renderPassContainer = std::make_unique<RenderPassContainer>(m_renderContext.get());

    ResourceScheduler::init(m_renderContext.get());
    Utils::init(m_renderContext.get());

    FE_LOG(LogRenderer, INFO, "Render context initialization completed.");
}

void Renderer::create_pipelines()
{
    m_pipelineManager->create_pipelines(m_renderPassContainer.get());
    m_imGuiRenderer->set_render_target_format(rhi::Format::B8G8R8A8_UNORM);
    m_imGuiRenderer->create_pipeline();
}

void Renderer::create_main_swap_chain()
{
    FE_LOG(LogRenderer, INFO, "Starting main swap chain initialization.");

    rhi::SwapChainInfo info;
    info.vSync = true;
    info.useHDR = false;
    info.colorSpace = rhi::ColorSpace::SRGB;
    info.format = rhi::Format::B8G8R8A8_UNORM;
    info.bufferCount = 3;
    info.window = m_window;
    rhi::create_swap_chain(&m_mainSwapChain, &info);
    m_renderContext->set_main_swap_chain(m_mainSwapChain);

    FE_LOG(LogRenderer, INFO, "Main swap chain initialization completed.");
}

void Renderer::acquire_next_image()
{
    m_syncManager->wait_fences();
    m_acquireSemaphore = m_syncManager->get_acquire_semaphore();
    rhi::Fence* fence = m_syncManager->get_fence();
    uint32 frameIndex = 0;
    rhi::acquire_next_image(m_mainSwapChain, m_acquireSemaphore, fence, &frameIndex);
    m_syncManager->wait_fences();
}

void Renderer::begin_frame()
{
    m_syncManager->begin_frame();
    m_commandManager->begin_frame();
    m_resourceManager->begin_frame();
    m_resourceLayoutTracker->begin_frame();

    m_bvhBuildSemaphore = nullptr;
    m_uploadSemaphore = nullptr;
    m_submitContexts.clear();
    m_pipelineBarriersByPassName.clear();

    if (g_frameNumber > 3)
        m_deletionQueue->destroy_objects();

    record_predraw_cmds();    
}

void Renderer::end_frame()
{
    m_commandManager->end_frame();
    m_resourceManager->end_frame();
    m_syncManager->end_frame();
    m_resourceManager->end_frame();

    if (g_frameIndex + 1 > m_mainSwapChain->bufferCount - 1) 
        g_frameIndex = 0;
    else
        ++g_frameIndex;

    rhi::set_frame_index(g_frameIndex);
}

void Renderer::schedule_frame()
{
    m_renderGraph->clear();
    m_resourceManager->begin_resource_scheduling();

    for (auto& [renderPasName, renderPass] : m_renderPassContainer->get_render_passes())
        renderPass->schedule_resources();
    m_resourceManager->end_resource_scheduling();
    m_renderGraph->build();
    m_resourceManager->allocate_scheduled_resources();
}

void Renderer::prepare_render_graph_execution()
{
    configure_submit_contexts();
    configure_pipeline_barriers();
    m_pipelineManager->wait_pipelines_creation();
}

void Renderer::execute_render_graph()
{
    record_worker_cmds();
    submit();
}

void Renderer::present()
{
    rhi::PresentInfo presentInfo;
    presentInfo.swapChains.push_back(m_mainSwapChain);
    presentInfo.waitSemaphores.push_back(m_backBufferSemaphore);

    rhi::present(&presentInfo);
}

void Renderer::configure_submit_contexts()
{
    std::vector<SubmitContext*> lastSubmitContextPerQueue(m_renderGraph->get_detected_queue_count(), nullptr);
    std::unordered_map<const RenderGraph::Node*, rhi::Semaphore*> signalSemaphoreByNode;
    bool requiresWaitingBVH = true;

    for (const RenderGraph::Node* node : m_renderGraph->get_nodes_in_global_exec_order())
    {
        uint32 nodeDependencyLevelIdx = node->get_dependency_level_index();
        uint32 nodeQueueIdx = node->get_queue_index();

        SubmitContext* lastSubmitContext = lastSubmitContextPerQueue.at(nodeQueueIdx);

        if (!lastSubmitContext
            || !node->get_nodes_to_sync_with().empty()
            || (node->useRayTracing && m_bvhBuildSemaphore && requiresWaitingBVH)
        )
        {
            // Invalidate the last submit context if a signal or wait semaphore needed to force the allocation of a new submit context
            lastSubmitContext = &m_submitContexts.emplace_back();
            lastSubmitContextPerQueue[nodeQueueIdx] = lastSubmitContext;
            lastSubmitContext->queueType = (rhi::QueueType)nodeQueueIdx;
        }

        if (node->is_sync_signal_required() && !lastSubmitContext->signalSemaphore)
        {            
            lastSubmitContext->signalSemaphore = m_syncManager->get_semaphore();
            rhi::set_name(lastSubmitContext->signalSemaphore, node->name().to_string());
            signalSemaphoreByNode[node] = lastSubmitContext->signalSemaphore;
        }

        if (!node->get_nodes_to_sync_with().empty())
        {
            for (const RenderGraph::Node* nodeToSync : node->get_nodes_to_sync_with())
            {
                lastSubmitContext->waitSemaphores.push_back(signalSemaphoreByNode.at(nodeToSync));
            }
        }

        if (node->useRayTracing && m_bvhBuildSemaphore && requiresWaitingBVH)
        {
            lastSubmitContext->waitSemaphores.push_back(m_bvhBuildSemaphore);
            requiresWaitingBVH = false;
        }

        if (lastSubmitContext->depencyLevelCommandContexts.empty() ||
            lastSubmitContext->depencyLevelCommandContexts.back().dependencyLevelIndex != nodeDependencyLevelIdx)
        {
            lastSubmitContext->depencyLevelCommandContexts.emplace_back(nodeDependencyLevelIdx);
        }

        lastSubmitContext->depencyLevelCommandContexts.back().nodesToRecord.push_back(node);
    }
}

void Renderer::configure_pipeline_barriers()
{
    // TODO: Need to add split barriers
    for (const RenderGraph::DependencyLevel& dependencyLevel : m_renderGraph->get_dependency_levels())
    {
        auto getViewReadLayotsInDependencyLevel = [&](
            RenderGraph::ViewName viewName, 
            const ResourceSchedulingInfo::RenderPassInfo& passInfo
        )
        {
            auto [resourceName, viewIndex] = RenderGraph::decode_view_name(viewName);

            rhi::ResourceLayout result = rhi::ResourceLayout::UNDEFINED;

            for (const RenderGraph::Node* node : dependencyLevel.get_nodes())
                if (node->get_read_views().contains(viewName))
                    result |= passInfo.viewInfos.at(viewIndex)->requestedLayout;

            return result;
        };

        for (const RenderGraph::Node* node : dependencyLevel.get_nodes())
        {
            auto addTransition = [&](RenderGraph::ViewName viewName, bool isReadDependency)
            {
                auto [resourceName, viewIndex] = RenderGraph::decode_view_name(viewName);
                
                if (resourceName == BACK_BUFFER_NAME)
                {
                    m_backBufferNode = node;
                    return;
                }
    
                RenderPassName passName = node->get_info().renderPassName;
                Resource* resource = m_resourceManager->get_resource(resourceName);
                const ResourceSchedulingInfo& schedulingInfo = resource->get_scheduling_info();
                const ResourceSchedulingInfo::RenderPassInfo* passInfo = schedulingInfo.get_render_pass_info(passName);
                FE_CHECK(passInfo);
    
                bool isResourceReadByMultipleQueue = 
                    isReadDependency && dependencyLevel.get_views_read_by_multiple_queues().contains(viewName);
                
                rhi::ResourceLayout newLayout = isResourceReadByMultipleQueue ?
                    getViewReadLayotsInDependencyLevel(viewName, *passInfo) :
                    passInfo->viewInfos.at(viewIndex)->requestedLayout;

                std::optional<rhi::PipelineBarrier> barrier = m_resourceLayoutTracker->get_transition_to_layout(resource, newLayout, viewIndex);
                if (barrier != std::nullopt)
                {
                    PipelineBarrierArray& passBarriers = m_pipelineBarriersByPassName[passName];
                    passBarriers.push_back(barrier.value());
                }

                // PIPELINE BARREIR REROUTING FROM COMPUTE TO GRAPHICS????????
            };

            for (RenderGraph::ViewName viewName : node->get_read_views())
                addTransition(viewName, true);
        
            for (RenderGraph::ViewName viewName : node->get_written_views())
                addTransition(viewName, false);
        }
    }
}

void Renderer::record_upload_cmd()
{
    rhi::CommandBuffer* cmd = m_commandManager->get_cmd(rhi::QueueType::GRAPHICS);
    rhi::begin_command_buffer(cmd);
    m_sceneManager->upload(cmd);
    rhi::end_command_buffer(cmd);

    m_uploadSubmitInfo.clear();
    m_uploadSubmitInfo.cmdBuffers.push_back(cmd);
    m_uploadSubmitInfo.signalSemaphores.push_back(m_uploadSemaphore);
    m_uploadSubmitInfo.waitSemaphores.push_back(m_acquireSemaphore);
}

void Renderer::record_bvh_build_cmd()
{
    m_bvhBuildSubmitInfo.queueType = rhi::QueueType::COMPUTE;

    rhi::CommandBuffer* cmd = m_commandManager->get_cmd(rhi::QueueType::COMPUTE);
    rhi::begin_command_buffer(cmd);
    m_sceneManager->build_bvh(cmd);
    rhi::end_command_buffer(cmd);

    m_bvhBuildSubmitInfo.clear();
    m_bvhBuildSubmitInfo.cmdBuffers.push_back(cmd);
    m_bvhBuildSubmitInfo.signalSemaphores.push_back(m_bvhBuildSemaphore);
    m_bvhBuildSubmitInfo.waitSemaphores.push_back(m_uploadSemaphore);
}

void Renderer::record_predraw_cmds()
{
    m_bvhBuildSemaphore = m_syncManager->get_semaphore();
    rhi::set_name(m_bvhBuildSemaphore, "BVHBuildSemaphore" + std::to_string(g_frameIndex));

    m_uploadSemaphore = m_syncManager->get_semaphore();
    rhi::set_name(m_uploadSemaphore, "UploadSemaphore" + std::to_string(g_frameIndex));
    
    TaskComposer::execute(m_commandRecordingTaskGroup, [&](TaskExecutionInfo)
    {
        record_upload_cmd();
        record_bvh_build_cmd();
    });
}

void Renderer::record_worker_cmds()
{
    TaskComposer::wait(m_commandRecordingTaskGroup);

    for (SubmitContext& submitContext : m_submitContexts)
    {
        for (DependencyLevelCommandContext& dependencyLevelContext : submitContext.depencyLevelCommandContexts)
        {
            TaskComposer::execute(m_commandRecordingTaskGroup, 
                [
                    this,
                    &dependencyLevelContext,
                    &submitContext
                ]
                (TaskExecutionInfo execInfo)
            {
                rhi::QueueType queueType = submitContext.queueType;

                dependencyLevelContext.workerCmd = m_commandManager->get_cmd(queueType);
                rhi::CommandBuffer* cmd = dependencyLevelContext.workerCmd;
                rhi::begin_command_buffer(cmd);

                for (const RenderGraph::Node* node : dependencyLevelContext.nodesToRecord)
                {
                    RenderPass* renderPass = m_renderPassContainer->get_render_pass(node->get_info().renderPassName);
                    FE_CHECK(renderPass);

                    const PipelineBarrierArray& barriers = m_pipelineBarriersByPassName[renderPass->get_name()];
                    rhi::add_pipeline_barriers(cmd, barriers);
                    rhi::SwapChain* usedSwapChain = nullptr;
                    bool requiresBeginRendering = queueType == rhi::QueueType::GRAPHICS && !node->useRayTracing;

                    if (requiresBeginRendering)
                    {
                        rhi::RenderingBeginInfo::Type type = m_backBufferNode == node 
                            ? rhi::RenderingBeginInfo::SWAP_CHAIN_PASS : rhi::RenderingBeginInfo::OFFSCREEN_PASS;
                        rhi::RenderingBeginInfo renderingBeginInfo(type);

                        renderPass->fill_rendering_begin_info(renderingBeginInfo);

                        if (type == rhi::RenderingBeginInfo::SWAP_CHAIN_PASS)
                        {
                            renderingBeginInfo.swapChainPass.swapChain = m_mainSwapChain;
                            usedSwapChain = m_mainSwapChain;
                        }

                        rhi::begin_rendering(cmd, &renderingBeginInfo);

                        if (m_backBufferNode == node)
                        {
                            submitContext.signalSemaphore = m_syncManager->get_semaphore();
                            m_backBufferSemaphore = submitContext.signalSemaphore;
                        }
                    }

                    renderPass->execute(cmd);

                    if (requiresBeginRendering)
                        rhi::end_rendering(cmd, usedSwapChain);

                }

                rhi::end_command_buffer(cmd);
            });
        }
    }
}

void Renderer::submit()
{
    TaskComposer::wait(m_commandRecordingTaskGroup);

    if (is_upload_cmd_submit_required())
        rhi::submit(&m_uploadSubmitInfo, m_syncManager->get_fence());

    if (is_bvh_build_cmd_submit_required())
        rhi::submit(&m_bvhBuildSubmitInfo, m_syncManager->get_fence());

    for (const SubmitContext& submitContext : m_submitContexts)
    {
        rhi::SubmitInfo submitInfo;
        submitInfo.queueType = submitContext.queueType;

        for (const DependencyLevelCommandContext& dependencyLevelContext : submitContext.depencyLevelCommandContexts)
            submitInfo.cmdBuffers.push_back(dependencyLevelContext.workerCmd);

        if (submitContext.signalSemaphore)
            submitInfo.signalSemaphores.push_back(submitContext.signalSemaphore);
        
        submitInfo.waitSemaphores = submitContext.waitSemaphores;

        rhi::submit(&submitInfo, m_syncManager->get_fence());
    }
}

bool Renderer::is_upload_cmd_submit_required() const
{
    return m_uploadSemaphore;
}

bool Renderer::is_bvh_build_cmd_submit_required() const
{
    return m_bvhBuildSemaphore;
}

}