#pragma once

#include "fwd.h"
#include "resources/resource_variant.h"

#include <array>
#include <string>

namespace fe::rhi
{

using PFN_Init = void(*)(const RHIInitInfo* initInfo);
using PFN_Cleanup = void(*)();

using PFN_CreateSwapChain = void(*)(SwapChain** swapChain, const SwapChainInfo* info);
using PFN_DestroySwapChain = void(*)(SwapChain* swapChain);

using PFN_CreateBuffer = void(*)(Buffer** buffer, const BufferInfo* info);
using PFN_UpdateBuffer = void(*)(Buffer* buffer, uint64 size, const void* data);
using PFN_DestroyBuffer = void(*)(Buffer* buffer);

using PFN_CreateTexture = void(*)(Texture** texture, const TextureInfo* info);
using PFN_DestroyTexture = void(*)(Texture* texture);
using PFN_CreateTextureView = void(*)(TextureView** textureView, const TextureViewInfo* info);
using PFN_DestroyTextureView = void(*)(TextureView* textureView);

using PFN_CreateBufferView = void(*)(BufferView** bufferView, const BufferViewInfo* info);
using PFN_DestroyBufferView = void(*)(BufferView* bufferView);

using PFN_CreateSampler = void(*)(Sampler** sampler, const SamplerInfo* info);
using PFN_DestroySampler = void(*)(Sampler* sampler);

using PFN_CreateShader = void(*)(Shader** shader, const ShaderInfo* info);
using PFN_DestroyShader = void(*)(Shader* shader);

using PFN_CreateGraphicsPipeline = void(*)(Pipeline** pipeline, const GraphicsPipelineInfo* info);
using PFN_CreateComputePipeline = void(*)(Pipeline** pipeline, const ComputePipelineInfo* info);
using PFN_CreateRayTracingPipeline = void(*)(Pipeline** pipeline, const RayTracingPipelineInfo* info);
using PFN_DestroyPipeline = void(*)(Pipeline* pipeline);

using PFN_CreateAccelerationStructure = void(*)(AccelerationStructure** accelerationStructure, const AccelerationStructureInfo* info);
using PFN_DestroyAccelerationStructure = void(*)(AccelerationStructure* accelerationStructure);
using PFN_WriteTopLevelAccelerationStructureInstance = void(*)(TLASInstance* instance, void* dst);
using PFN_WriteShaderIdentifier = void(*)(Pipeline* pipeline, uint32 groupIndex, void* dst);

using PFN_BindUniformBuffer = void(*)(Buffer* buffer, uint32 frameIndex, uint32 slot, uint32 size, uint32 offset);

using PFN_CreateCommandPool = void(*)(CommandPool** cmdPool, const CommandPoolInfo* info);
using PFN_DestroyCommandPool = void(*)(CommandPool* cmdPool);
using PFN_CreateCommandBuffer = void(*)(CommandBuffer** cmd, const CommandBufferInfo* info);
using PFN_DestroyCommandBuffer = void(*)(CommandBuffer* cmd);
using PFN_BeginCommandBuffer = void(*)(CommandBuffer* cmd);
using PFN_EndCommandBuffer = void(*)(CommandBuffer* cmd);
using PFN_ResetCommandPool = void(*)(CommandPool* cmdPool);

using PFN_CreateSemaphore = void(*)(Semaphore** semaphore);
using PFN_DestroySemaphore = void(*)(Semaphore* semaphore);
using PFN_CreateFence = void(*)(Fence** fence);
using PFN_DestroyFence = void(*)(Fence* fence);

using PFN_FillBuffer = void(*)(CommandBuffer* cmd, Buffer* dstBuffer, uint32 dstOffset, uint32 size, uint32 data);
using PFN_CopyBuffer = void(*)(CommandBuffer* cmd, Buffer* srcBuffer, Buffer* dstBuffer, uint32 size, uint32 srcOffset, uint32 dstOffset);
using PFN_InitTexture = void(*)(CommandBuffer* cmd, Texture* dstTexture, const TextureInitInfo* initInfo);
using PFN_CopyTexture = void(*)(CommandBuffer* cmd, Texture* srcTexture, Texture* dstTexture);
using PFN_CopyBufferToTexture = void(*)(CommandBuffer* cmd, Buffer* srcBuffer, Texture* dstTexture);
using PFN_CopyTextureToBuffer = void(*)(CommandBuffer* cmd, Texture* srcTexture, Buffer* dstBuffer);
using PFN_BlitTexture = void(*)(CommandBuffer* cmd, Texture* srcTexture, Texture* dstTexture, std::array<int32,3> srcOffset, std::array<int32,3> dstOffset, uint32 srcMipLevel, uint32 dstMipLevel, uint32 srcBaseLayer, uint32 dstBaseLayer);

using PFN_SetViewports = void(*)(CommandBuffer* cmd, const Viewport* viewports, uint32 viewportCount);
using PFN_SetScissors = void(*)(CommandBuffer* cmd, const Scissor* scissors, uint32 scissorCount);

using PFN_PushConstants = void(*)(CommandBuffer* cmd, Pipeline* pipeline, void* data);
using PFN_BindVertexBuffer = void(*)(CommandBuffer* cmd, Buffer* buffer);
using PFN_BindIndexBuffer = void(*)(CommandBuffer* cmd, Buffer* buffer, uint64 offset);
using PFN_BindPipeline = void(*)(CommandBuffer* cmd, Pipeline* pipeline);

using PFN_BuildAccelerationStructure = void(*)(CommandBuffer* cmd, const AccelerationStructure* dst, const AccelerationStructure* src);

using PFN_BeginRendering = void(*)(CommandBuffer* cmd, RenderingBeginInfo* beginInfo);
using PFN_EndRendering = void(*)(CommandBuffer* cmd, SwapChain* swapChain);

using PFN_Draw = void(*)(CommandBuffer* cmd, uint64 vertexCount);
using PFN_DrawIndexed = void(*)(CommandBuffer* cmd, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance);
using PFN_DrawIndirect = void(*)(CommandBuffer* cmd, Buffer* buffer, uint32 offset, uint32 drawCount, uint32 stride);
using PFN_DrawIndexedIndirect = void(*)(CommandBuffer* cmd, Buffer* buffer, uint32 offset, uint32 drawCount, uint32 stride);

using PFN_Dispatch = void(*)(CommandBuffer* cmd, uint32 groupCountX, uint32 groupCountY, uint32 groupCountZ);
using PFN_DispatchMesh = void(*)(CommandBuffer* cmd, uint32 groupCountX, uint32 groupCountY, uint32 groupCountZ);
using PFN_DispatchRays = void(*)(CommandBuffer* cmd, const DispatchRaysInfo* dispatchRaysInfo);

using PFN_AddPipelineBarriers = void(*)(CommandBuffer* cmd, const PipelineBarrier* barriers, uint32 barriersCount);

using PFN_AcquireNextImage = void(*)(SwapChain* swapChain, Semaphore* signalSemaphore, Fence* fence, uint32* frameIndex);
using PFN_Submit = void(*)(const SubmitInfo* submitInfo);
using PFN_Present = void(*)(const PresentInfo* presentInfo);
using PFN_WaitQueueIdle = void(*)(QueueType queueType);
using PFN_WaitForFences = void(*)(Fence* const* fences, uint32 fenceCount);

using PFN_GetAPI = API(*)();
using PFN_SetFrameIndex = void(*)(uint64 frameIndex);
using PFN_SetName = void(*)(ResourceVariant resource, const std::string& name);

using PFN_GetMinOffsetAlignment = uint64(*)(const BufferInfo* bufferInfo);
using PFN_GetGPUProperties = const GPUProperties&(*)();


}