#pragma once

#include "handle_base.h"
#include "renderer/rhi_types.h"

namespace fe::renderer
{

namespace detail
{

FE_DEFINE_RHI_RESOURCE_TRAITS(
    rhi::Pipeline, 
    rhi::create_graphics_pipeline, 
    rhi::destroy_pipeline
);

using PipelineBaseHandle = HandleBase<rhi::Pipeline, rhi::GraphicsPipelineInfo>;

}

class PipelineHandle : public detail::PipelineBaseHandle
{
    using Base = detail::PipelineBaseHandle;

public:
    using Base::Base;

    explicit PipelineHandle(const ComputePipelineCreateInfo& info)
    {
        rhi::create_compute_pipeline(&m_resource, &info);
    }

    explicit PipelineHandle(const RayTracingPipelineCreateInfo& info)
    {
        rhi::create_ray_tracing_pipeline(&m_resource, &info);
    }

    void init(const GraphicsPipelineCreateInfo& info)
    {
        reset();
        rhi::create_graphics_pipeline(&m_resource, &info);
    }

    void init(const ComputePipelineCreateInfo& info)
    {
        reset();
        rhi::create_compute_pipeline(&m_resource, &info);
    }

    void init(const RayTracingPipelineCreateInfo& info)
    {
        reset();
        rhi::create_ray_tracing_pipeline(&m_resource, &info);
    }
};

using PipelineRef = detail::RefBase<rhi::Pipeline>;

}