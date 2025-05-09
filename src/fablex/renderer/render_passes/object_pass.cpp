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
    
        rhi::bind_index_buffer(cmd, gpuModel.general_buffer(), gpuModel.index_offset());
        rhi::draw_indexed(cmd, gpuModel.index_count(), instanceCount, 0, 0, 0);
    });
}

}