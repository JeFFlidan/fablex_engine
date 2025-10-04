#pragma once

#include "internal.h"

namespace fe::rhi
{

struct Semaphore
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkSemaphore semaphore = VK_NULL_HANDLE;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()
};

struct Fence
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkFence fence = VK_NULL_HANDLE;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()
};

}