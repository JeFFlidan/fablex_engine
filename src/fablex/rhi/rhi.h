#pragma once

#include "resources.h"
#include <array>

namespace fe::rhi
{

struct RHIInitInfo
{
    GPUPreference gpuPreference{GPUPreference::DISCRETE};
    ValidationMode validationMode{ValidationMode::ENABLED};
};

inline void (*init)(const RHIInitInfo* initInfo);
inline void (*cleanup)();

inline void (*create_swap_chain)(SwapChain** swapChain, const SwapChainInfo* info);
inline void (*destroy_swap_chain)(SwapChain* swapChain);

inline void (*create_buffer)(Buffer** buffer, const BufferInfo* info);
inline void (*update_buffer)(Buffer*, uint64 size, const void* data);
inline void (*destroy_buffer)(Buffer* buffer);

inline void (*create_texture)(Texture** texture, const TextureInfo* info);
inline void (*destroy_texture)(Texture* texture);
inline void (*create_texture_view)(TextureView** textureView, const TextureViewInfo* info, const Texture* texture);
inline void (*destroy_texture_view)(TextureView* textureView);
inline void (*create_buffer_view)(BufferView** bufferView, const BufferViewInfo* info, const Buffer* buffer);
inline void (*destroy_buffer_view)(BufferView* bufferView);
inline void (*create_sampler)(Sampler** sampler, const SamplerInfo* info);
inline void (*destroy_sampler)(Sampler* sampler);
inline void (*create_shader)(Shader** shader, const ShaderInfo* info);
inline void (*destroy_shader)(Shader* shader);
inline void (*create_graphics_pipelines)(const std::vector<GraphicsPipelineInfo>& infos, std::vector<Pipeline*>& outPipelines);
inline void (*create_compute_pipelines)(const std::vector<ComputePipelineInfo>& infos, std::vector<Pipeline*>& outPipelines);
inline void (*destroy_pipeline)(Pipeline* pipeline);

inline void (*bind_uniform_buffer)(Buffer* buffer, uint32 frameIndex, uint32 slot, uint32 size, uint32 offset);

inline void (*create_command_pool)(CommandPool** cmdPool, const CommandPoolInfo* info);
inline void (*destroy_command_pool)(CommandPool* cmdPool);
inline void (*create_command_buffer)(CommandBuffer** cmd, const CommandBufferInfo* info);
inline void (*destroy_command_buffer)(CommandBuffer* cmd);
inline void (*begin_command_buffer)(CommandBuffer* cmd);
inline void (*end_command_buffer)(CommandBuffer* cmd);
inline void (*wait_command_buffer)(CommandBuffer* cmd1, CommandBuffer* cmd2);
inline void (*reset_command_pool)(CommandPool* cmdPool);

inline void (*create_semaphore)(Semaphore** semaphore);
inline void (*destroy_semaphore)(Semaphore* semaphore);
inline void (*create_fence)(Fence** fence);
inline void (*destroy_fence)(Fence* fence);

inline void (*fill_buffer)(CommandBuffer* cmd, Buffer* dstBuffer, uint32 dstOffset, uint32 size, uint32 data);
inline void (*copy_buffer)(CommandBuffer* cmd, Buffer* srcBuffer, Buffer* dstBuffer, uint32 size, uint32 srcOffset, uint32 dstOffset);
inline void (*copy_texture)(CommandBuffer* cmd, Texture* srcTexture, Texture* dstTexture);
inline void (*copy_buffer_to_texture)(CommandBuffer* cmd, Buffer* srcBuffer, Texture* dstTexture);
inline void (*copy_texture_to_buffer)(CommandBuffer* cmd, Texture* srcTexture, Buffer* dstBuffer);
inline void (*blit_texture)(
    CommandBuffer* cmd, 
    Texture* srcTexture, 
    Texture* dstTexture, 
    std::array<int32, 3> srcOffset, 
    std::array<int32, 3> dstOffset, 
    uint32 srcMipLevel, 
    uint32 dstMipLevel, 
    uint32 srcBaseLayer, 
    uint32 dstBaseLayer
);

inline void (*set_viewports)(CommandBuffer* cmd, const std::vector<Viewport>& viewports);
inline void (*set_scissors)(CommandBuffer* cmd, const std::vector<Scissor>& scissors);

inline void (*push_constants)(CommandBuffer* cmd, Pipeline* pipeline, void* data);
inline void (*bind_vertex_buffer)(CommandBuffer* cmd, Buffer* buffer);
inline void (*bind_index_buffer)(CommandBuffer* cmd, Buffer* buffer);
inline void (*bind_pipeline)(CommandBuffer* cmd, Pipeline* pipeline);

inline void (*begin_rendering)(CommandBuffer* cmd, RenderingBeginInfo* beginInfo);
inline void (*end_rendering)(CommandBuffer* cmd, SwapChain* swapChain);

inline void (*draw)(CommandBuffer* cmd, uint64 vertexCount);
inline void (*draw_indirect)(CommandBuffer* cmd, Buffer* buffer, uint32 offset, uint32 drawCount, uint32 stride);
inline void (*draw_indexed_indirect)(CommandBuffer* cmd, Buffer* buffer, uint32 offset, uint32 drawCount, uint32 stride);

inline void (*dispatch)(CommandBuffer* cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
inline void (*add_pipeline_barriers)(CommandBuffer* cmd, const std::vector<PipelineBarrier>& barriers);

inline void (*acquire_next_image)(SwapChain* swapChain, Semaphore* signalSemaphore, Fence* fence, uint32* frameIndex);
inline void (*submit)(SubmitInfo* submitInfo);
inline void (*present)(PresentInfo* presentInfo);
inline void (*wait_queue_idle)(QueueType queueType);
inline void (*wait_for_fences)(const std::vector<Fence*>& fences);

inline API (*get_api)();
inline uint32 (*get_frame_index)();

}