#include "triangle_pass.h"
#include "render_context.h"
#include "rhi/rhi.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(TrianglePass, RenderPass);

void TrianglePass::create_pipeline()
{
    create_graphics_pipeline();
}

void TrianglePass::execute(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd);

    set_default_viewport_and_scissor(cmd);

    bind_pipeline(cmd);

    rhi::draw(cmd, 3);
}

}