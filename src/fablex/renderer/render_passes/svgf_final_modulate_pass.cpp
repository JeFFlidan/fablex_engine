#include "svgf_final_modulate_pass.h"
#include "render_context.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(SVGFFinalModulatePass, RenderPass);

void SVGFFinalModulatePass::create_pipeline()
{
    create_compute_pipeline();
}

void SVGFFinalModulatePass::execute(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd);

    bind_pipeline(cmd);

    SVGFFinalModulatePushConstants pushConstants;
    fill_push_constants(pushConstants);

    push_constants(cmd, &pushConstants);
    dispatch(cmd, m_renderContext->render_surface(), {8, 8, 1});
}

}