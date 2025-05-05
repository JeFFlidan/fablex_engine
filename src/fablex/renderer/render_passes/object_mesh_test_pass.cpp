#include "object_mesh_test_pass.h"
#include "render_context.h"
#include "rhi/rhi.h"
#include "scene_manager/scene_manager.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(ObjectMeshTestPass, RenderPass);

void ObjectMeshTestPass::create_pipeline()
{
    create_graphics_pipeline([&](rhi::GraphicsPipelineInfo& info)
    {
        info.depthStencilState.isDepthTestEnabled = true;
        info.depthStencilState.isDepthWriteEnabled = true;
    });
}

void ObjectMeshTestPass::execute(rhi::CommandBuffer* cmd)
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

        rhi::dispatch_mesh(
            cmd, 
            gpuModel.get_thread_group_count_x(), 
            sceneManager->get_instance_count(gpuModel), 
            1
        );
    }
}


}