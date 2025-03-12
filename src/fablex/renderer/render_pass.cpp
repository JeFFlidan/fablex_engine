#include "render_pass.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(RenderPass, Object)

void RenderPass::init(const RenderPassMetadata& metadata, const RenderContext* renderContext)
{
    FE_CHECK(renderContext);
    m_renderContext = renderContext;
    m_info.name = metadata.renderPassName;
    m_info.type = metadata.type;
}

}