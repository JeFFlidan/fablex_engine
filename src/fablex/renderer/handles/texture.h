#pragma once

#include "handle_base.h"
#include "renderer/rhi_types.h"

namespace fe::renderer
{

namespace detail
{

FE_DEFINE_RHI_RESOURCE_TRAITS(
    rhi::Texture, 
    rhi::create_texture, 
    rhi::destroy_texture
);

}

using TextureHandle = detail::HandleBase<rhi::Texture, TextureViewCreateInfo>;
using TextureRef = detail::RefBase<rhi::Texture>;

}