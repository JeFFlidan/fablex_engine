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

    rhi::Viewport viewport;
    viewport.width = m_renderContext->get_render_surface().width;
    viewport.height = m_renderContext->get_render_surface().height;
    std::vector<rhi::Viewport> viewports = { viewport };
    rhi::set_viewports(cmd, viewports);

    rhi::Scissor scissor;
    scissor.right = (int32_t)m_renderContext->get_render_surface().width;
    scissor.bottom = (int32_t)m_renderContext->get_render_surface().height;
    std::vector<rhi::Scissor> scissors = { scissor };
    rhi::set_scissors(cmd, scissors);

    bind_pipeline(cmd);
    
    // TEST
    for (GPUModel* gpuModel : sceneManager->get_gpu_models())
    {
        rhi::dispatch_mesh(cmd, gpuModel->get_meshlet_count(), 1, 1);
    }
}


}