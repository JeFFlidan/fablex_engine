#include "object_mesh_test_pass.h"
#include "render_context.h"
#include "rhi/rhi.h"
#include "rhi/resources/graphics_pipeline_info.h"
#include "scene_manager/scene_manager.h"
#include "shaders/interops/shader_interop_push_constants.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(ObjectMeshTestPass, rg::RenderPass);

void ObjectMeshTestPass::create_pipeline()
{
    create_graphics_pipeline([&](GraphicsPipelineCreateInfo& info)
    {
        info.depthStencilState.isDepthTestEnabled = true;
        info.depthStencilState.isDepthWriteEnabled = true;
    });
}

void ObjectMeshTestPass::execute(CommandBufferRef cmd)
{
    FE_CHECK(cmd);

    SceneManager* sceneManager = m_renderContext->scene_manager();

    set_default_viewport_and_scissor(cmd);

    bind_pipeline(cmd);

    uint instanceOffset = 0;

    sceneManager->for_each_model([&](const GPUModel& gpuModel, uint32 modelIndex)
    {
        ObjectPushConstants pushConstants;
        pushConstants.modelIndex = modelIndex;
        pushConstants.instanceOffset = instanceOffset;
        push_constants(cmd, &pushConstants);
    
        uint32 instanceCount = gpuModel.instance_count();
        instanceOffset += instanceCount;
    
        cmd.dispatch_mesh({
            gpuModel.thread_group_count_x(), 
            instanceCount, 
            1
        });
    });
}


}