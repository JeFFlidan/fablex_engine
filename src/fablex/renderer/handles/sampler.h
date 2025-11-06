#pragma once

#include "handle_base.h"
#include "renderer/rhi_types.h"

namespace fe::renderer
{

FE_DEFINE_RHI_RESOURCE_RAII(
    Sampler, 
    rhi::create_sampler, 
    rhi::destroy_sampler
);

}
