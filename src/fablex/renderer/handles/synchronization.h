#pragma once

#include "handle_base.h"
#include "rhi/rhi.h"

namespace fe::renderer
{

namespace detail
{

FE_DEFINE_RHI_RESOURCE_TRAITS(
    rhi::Semaphore, 
    rhi::create_semaphore, 
    rhi::destroy_semaphore
);

FE_DEFINE_RHI_RESOURCE_TRAITS(
    rhi::Fence, 
    rhi::create_fence, 
    rhi::destroy_fence
);

}

using SemaphoreHandle = detail::HandleBase<rhi::Semaphore, void>;
using SemaphoreRef = detail::RefBase<rhi::Semaphore>;

using FenceHandle = detail::HandleBase<rhi::Fence, void>;
using FenceRef = detail::RefBase<rhi::Fence>;

}