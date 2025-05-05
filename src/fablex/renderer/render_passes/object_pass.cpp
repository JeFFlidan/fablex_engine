#include "object_pass.h"
#include "render_context.h"
#include "rhi/rhi.h"
#include "scene_manager/scene_manager.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(ObjectPass, RenderPass);

void ObjectPass::create_pipeline()
{
    create_graphics_pipeline([&](rhi::GraphicsPipelineInfo& info)
    {
        info.depthStencilState.isDepthTestEnabled = true;
        info.depthStencilState.isDepthWriteEnabled = true;
    });
}

void ObjectPass::execute(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd);

    SceneManager* sceneManager = m_renderContext->get_scene_manager();

    set_default_viewport_and_scissor(cmd);

    bind_pipeline(cmd);
    
    uint instanceOffset = 0;

    // TEST
    for (const GPUModel& gpuModel : sceneManager->get_gpu_models())
    {
        ObjectPushConstants pushConstants;
        pushConstants.modelIndex = sceneManager->get_model_index(gpuModel);
        pushConstants.instanceOffset = instanceOffset;
        push_constants(cmd, &pushConstants);

        uint32 instanceCount = sceneManager->get_instance_count(gpuModel);
        instanceOffset += instanceCount;

        rhi::bind_index_buffer(cmd, gpuModel.get_buffer(), gpuModel.get_index_offset());
        rhi::draw_indexed(cmd, gpuModel.get_index_count(), sceneManager->get_instance_count(gpuModel), 0, 0, 0);
    }
}

}