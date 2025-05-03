#pragma once

#include "common.h"
#include "core/object/object.h"
#include "pipeline_manager.h"

namespace fe::renderer
{

class RenderContext;

struct RenderPassInfo
{
    RenderPassName renderPassName;
    PipelineName pipelineName;
    RenderPassType type;
};

class RenderPass : public Object
{
    FE_DECLARE_OBJECT(RenderPass)

public:
    virtual ~RenderPass() = default;

    void init(const RenderPassMetadata& metadata, const RenderContext* renderContext);
    void schedule_resources();

    virtual void create_pipeline() { }
    virtual void execute(rhi::CommandBuffer* cmd) { }

    RenderPassInfo get_info() const;
    void fill_rendering_begin_info(rhi::RenderingBeginInfo& outBeginInfo) const;

    const RenderPassMetadata& get_metadata() const { return *m_metadata; }
    RenderPassName get_name() const { return m_metadata->renderPassName; }

protected:
    const RenderPassMetadata* m_metadata;
    const RenderContext* m_renderContext = nullptr;

    virtual void schedule_resources_internal() { }

    uint32 get_input_texture_descriptor(uint64 pushConstantsOffset, rhi::ViewType viewType, uint32 mipLevel = 0) const;
    uint32 get_output_storage_texture_descriptor(uint64 pushConstantOffset, uint32 mipLevel = 0) const;
    uint32 get_sampler_descriptor(ResourceName samplerName) const;
    
    void create_compute_pipeline();
    
    // Creates default graphics pipeline
    void create_graphics_pipeline();

    // Create graphics pipeline that will be configurated by callback
    void create_graphics_pipeline(const PipelineManager::GraphicsPipelineConfigurator& configurator);

    // Create default ray tracing pipeline
    void create_ray_tracing_pipeline();

    // Create ray tracing pipeline that will be configured by callback
    void create_ray_tracing_pipeline(const PipelineManager::RayTracingPipelineConfigurator& configurator);

    void bind_pipeline(rhi::CommandBuffer* cmd);
    void push_constants(rhi::CommandBuffer* cmd, void* data);
    void set_default_viewport_and_scissor(rhi::CommandBuffer* cmd) const;
    void set_viewport_and_scissor_by_window(rhi::CommandBuffer* cmd) const;
    void fill_dispatch_rays_info(rhi::DispatchRaysInfo& outInfo) const;

private:
    const PipelineMetadata& get_pipeline_metadata() const;
    const TextureMetadata& get_texture_metadata(ResourceName textureName) const;
    void fill_texture_info(const TextureMetadata& inMetadata, rhi::TextureInfo& outInfo) const;
};

}