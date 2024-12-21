#include "vulkan_rhi.h"
#include "rhi/rhi.h"
#include "rhi/resources.h"

#include <iostream>

namespace fe::rhi::vulkan
{

void init(const RHIInitInfo* initInfo)
{
    std::cout << "Vulkan is initialized" << std::endl;
}

void cleanup()
{

}

void create_swap_chain(SwapChain** swapChain, const SwapChainInfo* info)
{

}

void destroy_swap_chain(SwapChain* swapChain)
{

}

void create_buffer(Buffer** buffer, const BufferInfo* info)
{

}

void update_buffer(Buffer*, uint64 size, const void* data)
{

}

void destroy_buffer(Buffer* buffer)
{

}

void create_texture(Texture** texture, const TextureInfo* info)
{

}

void destroy_texture(Texture* texture)
{

}

void create_texture_view(TextureView** textureView, const TextureViewInfo* info, const Texture* texture)
{

}

void destroy_texture_view(TextureView* textureView)
{

}

void create_buffer_view(BufferView** bufferView, const BufferViewInfo* info)
{

}

void destroy_buffer_view(BufferView* bufferView)
{

}

void create_sampler(Sampler** sampler, const SamplerInfo* info)
{

}

void destroy_sampler(Sampler* sampler)
{

}

void create_shader(Shader** shader, const ShaderInfo* info)
{

}

void destroy_shader(Shader* shader)
{

}

void create_graphics_pipelines(const std::vector<GraphicsPipelineInfo>& infos, std::vector<Pipeline*>& outPipelines)
{

}

void create_compute_pipelines(const std::vector<ComputePipelineInfo>& infos, std::vector<Pipeline*>& outPipelines)
{

}

void destroy_pipeline(Pipeline* pipeline)
{

}

uint32 get_buffer_index(const Buffer* buffer)
{
    return 0;
}

uint32 get_texture_view_index(const TextureView* textureView)
{
    return 0;
}

uint32 get_buffer_view_index(const BufferView* bufferView)
{
    return 0;
}

uint32 get_sampler_index(const Sampler* sampler)
{
    return 0;
}

void bind_uniform_buffer(Buffer* buffer, uint32 slot, uint32 size, uint32 offset)
{
    
}

void create_command_pool(CommandPool** cmdPool, const CommandPoolInfo* info)
{

}

void destroy_command_pool(CommandPool* cmdPool)
{

}

void create_command_buffer(CommandBuffer** cmd, const CommandBufferInfo* info)
{

}

void destroy_command_buffer(CommandBuffer* cmd)
{

}

void begin_command_buffer(CommandBuffer* cmd)
{

}

void end_command_buffer(CommandBuffer* cmd)
{

}

void wait_command_buffer(CommandBuffer* cmd1, CommandBuffer* cmd2)
{

}

void reset_command_pool(CommandPool* cmdPool)
{

}

void create_semaphore(Semaphore** semaphore)
{

}

void destroy_semaphore(Semaphore* semaphore)
{

}

void create_fence(Fence** fence)
{

}

void destroy_fence(Fence* fence)
{

}

void fill_buffer(CommandBuffer* cmd, Buffer* dstBuffer, uint32 dstOffset, uint32 size, uint32 data)
{

}

void copy_buffer(CommandBuffer* cmd, Buffer* srcBuffer, Buffer* dstBuffer, uint32 size, uint32 srcOffset, uint32 dstOffset)
{

}

void copy_texture(CommandBuffer* cmd, Texture* srcTexture, Texture* dstTexture)
{

}

void copy_buffer_to_texture(CommandBuffer* cmd, Buffer* srcBuffer, Texture* dstTexture)
{

}

void copy_texture_to_buffer(CommandBuffer* cmd, Texture* srcTexture, Buffer* dstBuffer)
{

}

void blit_texture(
    CommandBuffer* cmd, 
    Texture* srcTexture, 
    Texture* dstTexture, 
    std::array<int32, 3> srcOffset, 
    std::array<int32, 3> dstOffset, 
    uint32 srcMipLevel, 
    uint32 dstMipLevel, 
    uint32 srcBaseLayer, 
    uint32 dstBaseLayer
)
{

}

void set_viewports(CommandBuffer* cmd, const std::vector<Viewport>& viewports)
{

}

void set_scissors(CommandBuffer* cmd, const std::vector<Scissor>& scissors)
{

}

void push_constants(CommandBuffer* cmd, Pipeline* pipeline, void* data)
{

}

void bind_vertex_buffer(CommandBuffer* cmd, Buffer* buffer)
{

}

void bind_index_buffer(CommandBuffer* cmd, Buffer* buffer)
{

}

void bind_pipeline(CommandBuffer* cmd, Buffer* buffer)
{

}

void begin_rendering(CommandBuffer* cmd, RenderingBeginInfo* beginInfo)
{

}

void end_rendering(CommandBuffer* cmd)
{

}

void draw(CommandBuffer* cmd, uint64 vertexCount)
{

}

void draw_indirect(CommandBuffer* cmd, Buffer* buffer, uint32 offset, uint32 drawCount, uint32 stride)
{

}

void draw_indexed_indirect(CommandBuffer* cmd, Buffer* buffer, uint32 offset, uint32 drawCount, uint32 stride)
{

}

void dispatch(CommandBuffer* cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{

}

void add_pipeline_barriers(CommandBuffer* cmd, uint32 barrierCount, const PipelineBarrier* barriers)
{

}

void acquire_next_image(SwapChain* swapChain, Semaphore* signalSemaphore, Fence* fence, uint32* imageIndex)
{

}

void submit(CommandBuffer* cmd, SubmitInfo* submitInfo)
{

}

void present(PresentInfo* presentInfo)
{

}

void wait_queue_idle(QueueType queueType)
{

}

void wait_for_fences(const std::vector<Fence*>& fences)
{

}

void fill_function_table()
{
    fe::rhi::init = init;
    fe::rhi::cleanup = cleanup;

    fe::rhi::create_swap_chain = create_swap_chain;
    fe::rhi::destroy_swap_chain = destroy_swap_chain;

    fe::rhi::create_buffer = create_buffer;
    fe::rhi::update_buffer = update_buffer;
    fe::rhi::destroy_buffer = destroy_buffer;

    fe::rhi::create_texture = create_texture;
    fe::rhi::destroy_texture = destroy_texture;
    fe::rhi::create_texture_view = create_texture_view;
    fe::rhi::destroy_texture_view = destroy_texture_view;
    fe::rhi::create_buffer_view = create_buffer_view;
    fe::rhi::destroy_buffer_view = destroy_buffer_view;
    fe::rhi::create_sampler = create_sampler;
    fe::rhi::destroy_sampler = destroy_sampler;
    fe::rhi::create_shader = create_shader;
    fe::rhi::destroy_shader = destroy_shader;
    fe::rhi::create_graphics_pipelines = create_graphics_pipelines;
    fe::rhi::create_compute_pipelines = create_compute_pipelines;
    fe::rhi::destroy_pipeline = destroy_pipeline;

    fe::rhi::get_buffer_index = get_buffer_index;
    fe::rhi::get_texture_view_index = get_texture_view_index;
    fe::rhi::get_buffer_view_index = get_buffer_view_index;
    fe::rhi::get_sampler_index = get_sampler_index;

    fe::rhi::bind_uniform_buffer = bind_uniform_buffer;

    fe::rhi::create_command_pool = create_command_pool;
    fe::rhi::destroy_command_pool = destroy_command_pool;
    fe::rhi::create_command_buffer = create_command_buffer;
    fe::rhi::destroy_command_buffer = destroy_command_buffer;
    fe::rhi::begin_command_buffer = begin_command_buffer;
    fe::rhi::end_command_buffer = end_command_buffer;
    fe::rhi::wait_command_buffer = wait_command_buffer;
    fe::rhi::reset_command_pool = reset_command_pool;

    fe::rhi::create_semaphore = create_semaphore;
    fe::rhi::destroy_semaphore = destroy_semaphore;
    fe::rhi::create_fence = create_fence;
    fe::rhi::destroy_fence = destroy_fence;

    fe::rhi::fill_buffer = fill_buffer;
    fe::rhi::copy_buffer = copy_buffer;
    fe::rhi::copy_texture = copy_texture;
    fe::rhi::copy_buffer_to_texture = copy_buffer_to_texture;
    fe::rhi::copy_texture_to_buffer = copy_texture_to_buffer;
    fe::rhi::blit_texture = blit_texture;

    fe::rhi::set_viewports = set_viewports;
    fe::rhi::set_scissors = set_scissors;
    
    fe::rhi::push_constants = push_constants;
    fe::rhi::bind_vertex_buffer = bind_vertex_buffer;
    fe::rhi::bind_index_buffer = bind_index_buffer;
    fe::rhi::bind_pipeline = bind_pipeline;

    fe::rhi::begin_rendering = begin_rendering;
    fe::rhi::end_rendering = end_rendering;

    fe::rhi::draw = draw;
    fe::rhi::draw_indirect = draw_indirect;
    fe::rhi::draw_indexed_indirect = draw_indexed_indirect;

    fe::rhi::dispatch = dispatch;
    fe::rhi::add_pipeline_barriers = add_pipeline_barriers;

    fe::rhi::acquire_next_image = acquire_next_image;
    fe::rhi::submit = submit;
    fe::rhi::present = present;
    fe::rhi::wait_queue_idle = wait_queue_idle;
    fe::rhi::wait_for_fences = wait_for_fences;
}

}