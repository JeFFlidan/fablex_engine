#pragma once

#include "handle_base.h"
#include "renderer/rhi_types.h"

namespace fe::renderer
{

FE_DEFINE_RHI_RESOURCE_RAII(
    Texture, 
    rhi::create_texture, 
    rhi::destroy_texture
);

}