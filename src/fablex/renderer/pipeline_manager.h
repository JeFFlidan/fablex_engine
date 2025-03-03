#pragma once

#include "common.h"
#include "rhi/resources.h"

#include <unordered_map>
#include <mutex>
#include <functional>
#include <variant>

namespace fe::renderer
{

struct PipelineMetadata;

class PipelineManager
{
public:
    using GraphicsPipelineConfigurator = std::function<void(rhi::GraphicsPipelineInfo&)>;
    using RayTracingPipelineConfigurator = std::function<void(rhi::RayTracingPipelineInfo&)>;

    void create_graphics_pipeline(const PipelineMetadata& pipelineMetadata);
    void create_graphics_pipeline(const PipelineMetadata& pipelineMetadata, const GraphicsPipelineConfigurator& configurator);
    void create_compute_pipeline(const PipelineMetadata& pipelineMetadata);
    void create_ray_tracing_pipeline(const PipelineMetadata& pipelineMetadata);
    void create_ray_tracing_pipeline(const PipelineMetadata& pipelineMetadata, const RayTracingPipelineConfigurator& configurator);
    
    void bind_pipeline(rhi::CommandBuffer* cmd, PipelineName name) const;
    rhi::Pipeline* get_pipeline(PipelineName name) const;

private:
    using PipelineInfoVariant = std::variant<rhi::GraphicsPipelineInfo*, rhi::ComputePipelineInfo*, rhi::RayTracingPipelineInfo*>;

    std::unordered_map<PipelineName, rhi::Pipeline*> m_pipelineByName;
    std::mutex m_mutex;

    void configure_pipeline_info(rhi::GraphicsPipelineInfo& outInfo, const PipelineMetadata& pipelineMetadata);
    void configure_pipeline_info(rhi::RayTracingPipelineInfo& outInfo, const PipelineMetadata& pipelineMetadata);
    void create_pipeline(PipelineName pipelineName, PipelineInfoVariant infoVariant);
};

}