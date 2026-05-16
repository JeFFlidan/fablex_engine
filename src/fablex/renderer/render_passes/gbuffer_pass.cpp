#include "gbuffer_pass.h"
#include "render_context.h"
#include "shaders/interops/shader_interop_push_constants.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(GBufferPass, RenderPass);

void GBufferPass::create_pipeline()
{
    create_pipelines();
}

void GBufferPass::execute(CommandBufferRef cmd)
{
    FE_CHECK(cmd);

    bind_pipeline(cmd);

    GBufferPushConstants pushConstants;
    fill_push_constants(pushConstants);

    push_constants(cmd, &pushConstants, 0);
    dispatch(cmd, m_renderContext->render_surface(), {4, 4, 1});
}

}