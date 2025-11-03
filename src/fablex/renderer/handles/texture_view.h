#pragma once

#include "handle_base.h"
#include "renderer/rhi_types.h"

namespace fe::renderer
{

namespace detail
{

FE_DEFINE_RHI_RESOURCE_TRAITS(
    rhi::TextureView, 
    rhi::create_texture_view, 
    rhi::destroy_texture_view
)

}

using TextureViewHandle = detail::HandleBase<rhi::TextureView, TextureViewCreateInfo>;
using TextureViewRef = detail::RefBase<rhi::TextureView>;

}