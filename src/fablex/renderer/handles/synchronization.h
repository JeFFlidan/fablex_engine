#pragma once

#include "handle_base.h"
#include "rhi/rhi.h"

namespace fe::renderer
{

FE_DEFINE_RHI_RESOURCE_RAII_NO_CREATE_INFO(
    Semaphore, 
    rhi::create_semaphore, 
    rhi::destroy_semaphore
);

FE_DEFINE_RHI_RESOURCE_RAII_NO_CREATE_INFO(
    Fence, 
    rhi::create_fence, 
    rhi::destroy_fence
);

}