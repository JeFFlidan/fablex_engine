#pragma once

#include "common.h"
#include "shader_identifiers.h"
#include "handles/pipeline.h"
#include "handles/command_buffer.h"

#include "core/task_types.h"

#include <unordered_map>
#include <mutex>
#include <functional>
#include <variant>

namespace fe::renderer
{

class PipelineManager
{
public:
    using GraphicsPipelineConfigurator = std::function<void(GraphicsPipelineCreateInfo&)>;
    using RayTracingPipelineConfigurator = std::function<void(RayTracingPipelineCreateInfo&)>;

    PipelineManager(ShaderManager* shaderManager);
    ~PipelineManager();

    void create_graphics_pipeline(const rg::PipelineMetadata& pipelineMetadata);
    void create_graphics_pipeline(const rg::PipelineMetadata& pipelineMetadata, const GraphicsPipelineConfigurator& configurator);
    void create_compute_pipeline(const rg::PipelineMetadata& pipelineMetadata);
    void create_ray_tracing_pipeline(const rg::PipelineMetadata& pipelineMetadata);
    void create_ray_tracing_pipeline(const rg::PipelineMetadata& pipelineMetadata, const RayTracingPipelineConfigurator& configurator);
    
    // Create pipeline for all passes in container using multithreading. Wait function must be called manually. 
    void create_pipelines(rg::RenderPassContainer* renderPassContainer);
    void wait_pipelines_creation();
    
    void bind_pipeline(CommandBufferRef cmd, PipelineName name) const;
    void push_constants(CommandBufferRef cmd, PipelineName name, void* data) const;
    void fill_dispatch_rays_info(PipelineName name, DispatchRaysInfo& outInfo) const;  // Only for simple RT pipelines
    PipelineRef get_pipeline(PipelineName name) const;

private:
    using PipelineInfoVariant = std::variant<GraphicsPipelineCreateInfo*, ComputePipelineCreateInfo*, RayTracingPipelineCreateInfo*>;

    std::unordered_map<PipelineName, PipelineHandle> m_pipelineByName;
    std::unordered_map<PipelineName, ShaderIdentifiers> m_shaderIdentifiersByName;  // Only for RT pipelines
    std::mutex m_pipelineMapMutex;
    std::mutex m_shaderIdentifiersMapMutex;
    ShaderManager* m_shaderManager = nullptr;
    TaskGroup* m_taskGroup = nullptr;

    void configure_pipeline_info(GraphicsPipelineCreateInfo& outInfo, const rg::PipelineMetadata& pipelineMetadata);
    void configure_pipeline_info(RayTracingPipelineCreateInfo& outInfo, const rg::PipelineMetadata& pipelineMetadata);
    void create_pipeline(PipelineName pipelineName, PipelineInfoVariant infoVariant);
};

}