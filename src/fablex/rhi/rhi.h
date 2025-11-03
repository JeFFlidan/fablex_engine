#pragma once

#include "pfn.h"

namespace fe::rhi
{

inline PFN_Init init = nullptr;
inline PFN_Cleanup cleanup = nullptr;

inline PFN_CreateSwapChain create_swap_chain = nullptr;
inline PFN_DestroySwapChain destroy_swap_chain = nullptr;

inline PFN_CreateBuffer create_buffer = nullptr;
inline PFN_UpdateBuffer update_buffer = nullptr;
inline PFN_DestroyBuffer destroy_buffer = nullptr;

inline PFN_CreateTexture create_texture = nullptr;
inline PFN_DestroyTexture destroy_texture = nullptr;
inline PFN_CreateTextureView create_texture_view = nullptr;
inline PFN_DestroyTextureView destroy_texture_view = nullptr;

inline PFN_CreateBufferView create_buffer_view = nullptr;
inline PFN_DestroyBufferView destroy_buffer_view = nullptr;

inline PFN_CreateSampler create_sampler = nullptr;
inline PFN_DestroySampler destroy_sampler = nullptr;

inline PFN_CreateShader create_shader = nullptr;
inline PFN_DestroyShader destroy_shader = nullptr;

inline PFN_CreateGraphicsPipeline create_graphics_pipeline = nullptr;
inline PFN_CreateComputePipeline create_compute_pipeline = nullptr;
inline PFN_CreateRayTracingPipeline create_ray_tracing_pipeline = nullptr;
inline PFN_DestroyPipeline destroy_pipeline = nullptr;

inline PFN_CreateAccelerationStructure create_acceleration_structure = nullptr;
inline PFN_DestroyAccelerationStructure destroy_acceleration_structure = nullptr;
inline PFN_WriteTopLevelAccelerationStructureInstance write_top_level_acceleration_structure_instance = nullptr;
inline PFN_WriteShaderIdentifier write_shader_identifier = nullptr;

inline PFN_BindUniformBuffer bind_uniform_buffer = nullptr;

inline PFN_CreateCommandPool create_command_pool = nullptr;
inline PFN_DestroyCommandPool destroy_command_pool = nullptr;
inline PFN_CreateCommandBuffer create_command_buffer = nullptr;
inline PFN_DestroyCommandBuffer destroy_command_buffer = nullptr;
inline PFN_BeginCommandBuffer begin_command_buffer = nullptr;
inline PFN_EndCommandBuffer end_command_buffer = nullptr;
inline PFN_ResetCommandPool reset_command_pool = nullptr;

inline PFN_CreateSemaphore create_semaphore = nullptr;
inline PFN_DestroySemaphore destroy_semaphore = nullptr;
inline PFN_CreateFence create_fence = nullptr;
inline PFN_DestroyFence destroy_fence = nullptr;

inline PFN_FillBuffer fill_buffer = nullptr;
inline PFN_CopyBuffer copy_buffer = nullptr;
inline PFN_InitTexture init_texture = nullptr;
inline PFN_CopyTexture copy_texture = nullptr;
inline PFN_CopyBufferToTexture copy_buffer_to_texture = nullptr;
inline PFN_CopyTextureToBuffer copy_texture_to_buffer = nullptr;
inline PFN_BlitTexture blit_texture = nullptr;

inline PFN_SetViewports set_viewports = nullptr;
inline PFN_SetScissors set_scissors = nullptr;

inline PFN_PushConstants push_constants = nullptr;
inline PFN_BindVertexBuffer bind_vertex_buffer = nullptr;
inline PFN_BindIndexBuffer bind_index_buffer = nullptr;
inline PFN_BindPipeline bind_pipeline = nullptr;

inline PFN_BuildAccelerationStructure build_acceleration_structure = nullptr;

inline PFN_BeginRendering begin_rendering = nullptr;
inline PFN_EndRendering end_rendering = nullptr;

inline PFN_Draw draw = nullptr;
inline PFN_DrawIndexed draw_indexed = nullptr;
inline PFN_DrawIndirect draw_indirect = nullptr;
inline PFN_DrawIndexedIndirect draw_indexed_indirect = nullptr;

inline PFN_Dispatch dispatch = nullptr;
inline PFN_DispatchMesh dispatch_mesh = nullptr;
inline PFN_DispatchRays dispatch_rays = nullptr;
inline PFN_AddPipelineBarriers add_pipeline_barriers = nullptr;

inline PFN_AcquireNextImage acquire_next_image = nullptr;
inline PFN_Submit submit = nullptr;
inline PFN_Present present = nullptr;
inline PFN_WaitQueueIdle wait_queue_idle = nullptr;
inline PFN_WaitForFences wait_for_fences = nullptr;

inline PFN_GetAPI get_api = nullptr;
inline PFN_SetFrameIndex set_frame_index = nullptr;
inline PFN_SetName set_name = nullptr;

inline PFN_GetMinOffsetAlignment get_min_offset_alignment = nullptr;
inline PFN_GetGPUProperties get_gpu_properties = nullptr;

}