#pragma once

#include "common.h"
#include "core/object/object.h"
#include "pipeline_manager.h"

namespace fe::renderer
{

class RenderContext;
class SceneManager;
class RenderGraphMetadata;
struct RenderSurface;
struct RenderPassMetadata;
struct TextureMetadata;

struct RenderPassInfo
{
    RenderPassName renderPassName;
    PipelineName pipelineName;
    RenderPassType type;
};

using DispatchSizes = std::array<uint32, 3>;

class RenderPass : public Object
{
    FE_DECLARE_OBJECT(RenderPass)

public:
    virtual ~RenderPass() = default;

    void init(const RenderPassMetadata& metadata, const RenderContext* renderContext);
    virtual void schedule_resources();

    virtual void create_pipeline() { }
    virtual void execute(rhi::CommandBuffer* cmd) { }

    RenderPassInfo get_info() const;
    void fill_rendering_begin_info(rhi::RenderingBeginInfo& outBeginInfo) const;

    RenderPassName get_name() const;
    const RenderPassMetadata& get_metadata() const { return *m_metadata; }

protected:
    const RenderPassMetadata* m_metadata;
    const RenderContext* m_renderContext = nullptr;

    void create_compute_pipeline();
    
    // Creates default graphics pipeline
    void create_graphics_pipeline();

    // Create graphics pipeline that will be configurated by callback
    void create_graphics_pipeline(const PipelineManager::GraphicsPipelineConfigurator& configurator);

    // Create default ray tracing pipeline
    void create_ray_tracing_pipeline();

    // Create ray tracing pipeline that will be configured by callback
    void create_ray_tracing_pipeline(const PipelineManager::RayTracingPipelineConfigurator& configurator);

    SceneManager* scene_manager() const;

    void bind_pipeline(rhi::CommandBuffer* cmd);
    void push_constants(rhi::CommandBuffer* cmd, void* data);
    void set_default_viewport_and_scissor(rhi::CommandBuffer* cmd) const;
    void set_viewport_and_scissor_by_window(rhi::CommandBuffer* cmd) const;
    void fill_dispatch_rays_info(rhi::DispatchRaysInfo& outInfo) const;
    void dispatch(rhi::CommandBuffer* cmd, const RenderSurface& surface, const DispatchSizes& groupSizes);

    template<typename T>
    void fill_push_constants(T& pushConstants)
    {
        fill_push_constants(T::TypeName, &pushConstants);
    }

private:
    // Resource names for corss frame read
    using ResourceNamesXFR = std::pair<ResourceName, ResourceName>;

    const RenderGraphMetadata& get_render_graph_metadata() const;
    const PipelineMetadata& get_pipeline_metadata() const;
    const TextureMetadata& get_texture_metadata(ResourceName textureName) const;
    void fill_texture_info(const TextureMetadata& inMetadata, rhi::TextureInfo& outInfo) const;
    ResourceNamesXFR get_resource_names_xfr(ResourceName baseName) const;
    ResourceName get_prev_frame_resource_name(ResourceName baseName) const;
    ResourceName get_curr_frame_resource_name(ResourceName baseName) const;

    void fill_push_constants(PushConstantsName pushConstantsName, void* data) const;
};

}