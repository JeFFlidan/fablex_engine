#include "svgf_pass.h"
#include "render_context.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(SVGFPass, RenderPass);

void SVGFPass::create_pipeline()
{
    create_pipelines();
}

void SVGFPass::execute(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd);

    bind_pipeline(cmd, 0);

    SVGFFilterMomentsPushConstants pushConstants;
    pushConstants.phiColor = 10.0f;
    pushConstants.phiNormal = 128.0f;

    fill_push_constants(pushConstants);

    push_constants(cmd, &pushConstants, 0);
    dispatch(cmd, m_renderContext->render_surface(), {8, 8, 1});

    bind_pipeline(cmd, 1);

    for (uint32 i = 0; i != 4; ++i)
    {
        SVGFAtrousPushConstants pushConstants;
        pushConstants.phiNormal = 128.0f;
        pushConstants.phiColor = 10.0f;
        pushConstants.stepSize = 1 << i;

        fill_push_constants(pushConstants);

        push_constants(cmd, &pushConstants, 1);
        dispatch(cmd, m_renderContext->render_surface(), {8, 8, 1});
    }
}

}