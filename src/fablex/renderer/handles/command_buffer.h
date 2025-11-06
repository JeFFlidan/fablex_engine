#pragma once

#include "buffer.h"
#include "texture.h"
#include "pipeline.h"
#include "swap_chain.h"
#include "acceleration_structure.h"
#include "renderer/rhi_types.h"
#include "rhi/resources/viewport.h"
#include "rhi/resources/pipeline_barrier.h"
#include <vector>

namespace fe::renderer
{

struct DispatchSizes
{
    uint32 x = 1;
    uint32 y = 1;
    uint32 z = 1;
};

namespace detail
{

template<typename Base>
class CommandBufferInterface : public Base
{
public:
    using Base::Base;

    void begin_recording()
    {
        rhi::begin_command_buffer(this->m_resource);
    }

    void end_recording()
    {
        rhi::end_command_buffer(this->m_resource);
    }
    
    void fill_buffer(BufferRef dstBuffer, uint32 dstOffset, uint32 size, uint32 data)
    {
        rhi::fill_buffer(this->m_resource, dstBuffer, dstOffset, size, data);
    }

    // If size = 0, srcBuffer size will be taken
    void copy_buffer(BufferRef srcBuffer, BufferRef dstBuffer, uint32 size = 0, uint32 srcOffset = 0, uint32 dstOffset = 0)
    {
        rhi::copy_buffer(this->m_resource, srcBuffer, dstBuffer, size ? size : srcBuffer->size, srcOffset, dstOffset);
    }

    void init_texture(TextureRef dstTexture, const TextureInitInfo& initInfo)
    {
        rhi::init_texture(this->m_resource, dstTexture, &initInfo);
    }

    void copy_texture(TextureRef srcTexture, TextureRef dstTexture)
    {
        rhi::copy_texture(this->m_resource, srcTexture, dstTexture);
    }

    void copy_buffer_to_texture(BufferRef srcBuffer, TextureRef dstTexture)
    {
        rhi::copy_buffer_to_texture(this->m_resource, srcBuffer, dstTexture);
    }

    void copy_texture_to_buffer(TextureRef srcTexture, BufferRef dstBuffer)
    {
        rhi::copy_texture_to_buffer(this->m_resource, srcTexture, dstBuffer);
    }

    void blit_texture(
        TextureRef srcTexture, 
        TextureRef dstTexture,
        std::array<int32, 3> srcOffset, 
        std::array<int32, 3> dstOffset,
        uint32 srcMipLevel, 
        uint32 dstMipLevel, 
        uint32 srcBaseLayer, 
        uint32 dstBaseLayer
    )
    {
        rhi::blit_texture(
            this->m_resource, 
            srcTexture, 
            dstTexture, 
            srcOffset, 
            dstOffset,
            srcMipLevel, 
            dstMipLevel, 
            srcBaseLayer, 
            dstBaseLayer
        );
    }

    void set_viewports(const std::vector<rhi::Viewport>& viewports)
    {
        rhi::set_viewports(this->m_resource, viewports.data(), viewports.size());
    }

    void set_scissors(const std::vector<rhi::Scissor>& scissors)
    {
        rhi::set_scissors(this->m_resource, scissors.data(), scissors.size());
    }

    void push_constants(PipelineRef pipeline, void* data)
    {
        rhi::push_constants(this->m_resource, pipeline, data);
    }

    void bind_vertex_buffer(BufferRef buffer)
    {
        rhi::bind_vertex_buffer(this->m_resource, buffer);
    }

    void bind_index_buffer(BufferRef buffer, uint64 offset)
    {
        rhi::bind_index_buffer(this->m_resource, buffer, offset);
    }

    void bind_pipeline(PipelineRef pipeline)
    {
        rhi::bind_pipeline(this->m_resource, pipeline);
    }

    void build_tlas(TLASRef dst, TLASRef src)
    {
        rhi::build_acceleration_structure(this->m_resource, dst, src);
    }

    void build_blas(BLASRef dst, BLASRef src)
    {
        rhi::build_acceleration_structure(this->m_resource, dst, src);
    }

    void begin_rendering(RenderingBeginInfo* beginInfo)
    {
        rhi::begin_rendering(this->m_resource, beginInfo);
    }

    void end_rendering(SwapChainRef swapChain)
    {
        rhi::end_rendering(this->m_resource, swapChain);
    }

    void draw(uint64 vertexCount)
    {
        rhi::draw(this->m_resource, vertexCount);
    }

    void draw_indexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance)
    {
        rhi::draw_indexed(this->m_resource, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void draw_indirect(BufferRef buffer, uint32 offset, uint32 drawCount, uint32 stride)
    {
        rhi::draw_indirect(this->m_resource, buffer, offset, drawCount, stride);
    }

    void draw_indexed_indirect(BufferRef buffer, uint32 offset, uint32 drawCount, uint32 stride)
    {
        rhi::draw_indexed_indirect(this->m_resource, buffer, offset, drawCount, stride);
    }

    void dispatch(const DispatchSizes& dispatchSizes)
    {
        rhi::dispatch(this->m_resource, dispatchSizes.x, dispatchSizes.y, dispatchSizes.z);
    }

    void dispatch_mesh(const DispatchSizes& dispatchSizes)
    {
        rhi::dispatch_mesh(this->m_resource, dispatchSizes.x, dispatchSizes.y, dispatchSizes.z);
    }

    void dispatch_rays(const DispatchRaysInfo* dispatchRaysInfo)
    {
        rhi::dispatch_rays(this->m_resource, dispatchRaysInfo);
    }

    void add_pipeline_barriers(const std::vector<PipelineBarrier>& barriers)
    {
        rhi::add_pipeline_barriers(this->m_resource, barriers.data(), barriers.size());
    }

    void begin_event(const std::string& eventName)
    {
        rhi::begin_event(this->m_resource, eventName);
    }

    void end_event()
    {
        rhi::end_event(this->m_resource);
    }

    void set_marker(const std::string& markerName)
    {
        rhi::set_marker(this->m_resource, markerName);
    }
};

}

FE_DEFINE_RHI_RESOURCE_RAII_EXTENDED(
    CommandBuffer, 
    rhi::create_command_buffer, 
    rhi::destroy_command_buffer
);

}