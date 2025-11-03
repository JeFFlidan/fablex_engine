#include "rendering_infos.h"
#include "resource_metadata.h"
#include "texture.h"
#include "rhi/utils.h"

namespace fe::renderer::rg
{

void OffscreenPassRenderingInfo::add_render_target(
    const Texture& texture, 
    const TextureMetadata& textureMetadata,
    const RenderTargetMetadata& renderTargetMetadata
)
{
    rhi::RenderTarget& renderTarget = m_info.offscreenPass.renderTargets.emplace_back();
    
    if (rhi::is_depth_stencil_format(textureMetadata.format))
        renderTarget.target = texture.dsv();
    else
        renderTarget.target = texture.rtv();

    FE_CHECK(renderTarget.target);

    renderTarget.clearValue = renderTargetMetadata.clearValues;
    renderTarget.loadOp = renderTargetMetadata.loadOp;
    renderTarget.storeOp = renderTargetMetadata.storeOp;
}

void SwapChainPassRenderingInfo::set_render_target(const RenderTargetMetadata& renderTargetMetadata)
{
    m_info.swapChainPass.clearValues = renderTargetMetadata.clearValues;
}

}