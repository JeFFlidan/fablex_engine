#include "swap_chain_pass.h"
#include "render_context.h"
#include "imgui_renderer.h"
#include "rhi/rhi.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(SwapChainPass, RenderPass);

void SwapChainPass::create_pipeline()
{
    create_graphics_pipeline([&](rhi::GraphicsPipelineInfo& info)
    {
        info.colorAttachmentFormats.clear();
        info.colorAttachmentFormats.push_back(rhi::Format::B8G8R8A8_UNORM);
    });
}

void SwapChainPass::execute(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd);
    
    SwapChainPushConstants pushConstants;
    fill_push_constants(pushConstants);

    m_renderContext->imgui_renderer()->set_viewport_texture_descriptor(pushConstants.resultTexture.descriptor);
    m_renderContext->imgui_renderer()->draw(cmd);
    
    // set_viewport_and_scissor_by_window(cmd);
    // bind_pipeline(cmd);
    // push_constants(cmd, &pushConstants);
    // rhi::draw(cmd, 6);
}

}