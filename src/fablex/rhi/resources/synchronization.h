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

    struct D3D12
    {
#if defined(FE_D3D12)
        static constexpr uint64 INVALID_VALUE = ~0u;
        ID3D12Fence* fence = nullptr;
        HANDLE waitIdleFenceEvent = nullptr;
        uint64 fenceValue = INVALID_VALUE;
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

    struct D3D12
    {
#if defined(FE_D3D12)
        static constexpr uint64 INVALID_VALUE = ~0u;
        ID3D12Fence* fence = nullptr;
        HANDLE waitIdleFenceEvent = nullptr;
        uint64 fenceValue = INVALID_VALUE;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()
};

}