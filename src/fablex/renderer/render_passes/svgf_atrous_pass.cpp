#include "svgf_atrous_pass.h"
#include "render_context.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(SVGFAtrousPass, RenderPass);

void SVGFAtrousPass::create_pipeline()
{
    create_compute_pipeline();
}

void SVGFAtrousPass::execute(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd);

    bind_pipeline(cmd);

    SVGFAtrousPushConstants pushConstants;
    fill_push_constants(pushConstants);

    push_constants(cmd, &pushConstants);
    dispatch(cmd, m_renderContext->render_surface(), {8, 8, 1});
}

}