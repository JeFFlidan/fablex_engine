#include "svgf_filter_moments_pass.h"
#include "render_context.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(SVGFFilterMomentsPass, RenderPass);

void SVGFFilterMomentsPass::create_pipeline()
{
    create_compute_pipeline();
}

void SVGFFilterMomentsPass::execute(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd);

    bind_pipeline(cmd);

    SVGFFilterMomentsPushConstants pushConstants;
    pushConstants.phiColor = 10.0f;
    pushConstants.phiNormal = 128.0f;
    fill_push_constants(pushConstants);

    push_constants(cmd, &pushConstants);
    dispatch(cmd, m_renderContext->render_surface(), {8, 8, 1});
}

}