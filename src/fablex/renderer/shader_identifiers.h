#pragma once

#include "handles/buffer.h"
#include "handles/pipeline.h"
#include "rhi/resources/ray_tracing.h"

namespace fe::renderer
{

class ShaderIdentifiers
{
public:
    void init(PipelineRef pipeline, const RayTracingPipelineCreateInfo& createInfo);
    void fill_dispatch_rays_info(DispatchRaysInfo& outRaysInfo) const;

    bool is_valid() const { return m_buffer; }

private:
    BufferHandle m_buffer;

    rhi::ShaderIdentifier raygenIdentifier;
    rhi::ShaderIdentifier missIdentifier;
    rhi::ShaderIdentifier hitGroupIdentifier;
    rhi::ShaderIdentifier callableIdentifier;
};

}