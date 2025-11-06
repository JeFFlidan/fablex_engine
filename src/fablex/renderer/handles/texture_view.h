#pragma once

#include "handle_base.h"
#include "renderer/rhi_types.h"

namespace fe::renderer
{

FE_DEFINE_RHI_RESOURCE_RAII(
    TextureView, 
    rhi::create_texture_view, 
    rhi::destroy_texture_view
);

}