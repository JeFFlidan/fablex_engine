#include "rng_seed_generation_pass.h"
#include "globals.h"
#include "render_context.h"
#include "scene_manager/scene_manager.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(RNGSeedGenerationPass, RenderPass);

void RNGSeedGenerationPass::create_pipeline()
{
    create_compute_pipeline();
}

void RNGSeedGenerationPass::execute(rhi::CommandBuffer* cmd)
{
    bind_pipeline(cmd);

    const GPUTexture& blueNoise = scene_manager()->blue_noise_texture();
    
    RNGSeedGenerationPushConstants pushConstants;
    fill_push_constants(pushConstants);

    pushConstants.blueNoiseTextureSize = blueNoise.texture()->width;
    pushConstants.blueNoiseTextureDepth = blueNoise.texture()->depth;
    pushConstants.frameNumber = g_frameNumber;
    
    push_constants(cmd, &pushConstants);
    dispatch(cmd, m_renderContext->render_surface(), {8, 8, 1});
}

}