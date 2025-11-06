#pragma once

#include "handle_base.h"
#include "renderer/rhi_types.h"

namespace fe::renderer
{

FE_DEFINE_RHI_RESOURCE_RAII(
    BufferView, 
    rhi::create_buffer_view, 
    rhi::destroy_buffer_view
);

}