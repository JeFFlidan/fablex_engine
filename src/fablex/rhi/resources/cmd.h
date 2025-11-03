#pragma once

#include "internal.h"

namespace fe::rhi
{

struct Semaphore;
struct Fence;
struct SwapChain;

struct CommandPoolInfo
{
    QueueType queueType;
};

struct CommandPool
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkCommandPool cmdPool = VK_NULL_HANDLE;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()

    QueueType queueType;
};

struct CommandBufferInfo
{
    CommandPool* cmdPool = nullptr;
};

struct CommandBuffer
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()

    const CommandPool* cmdPool = nullptr;
};

struct SubmitInfo
{
    QueueType queueType = QueueType::GRAPHICS;
    std::vector<CommandBuffer*> cmdBuffers;
    std::vector<Semaphore*> waitSemaphores;
    std::vector<Semaphore*> signalSemaphores;
    Fence* signalFence = nullptr;

    void clear()
    {
        cmdBuffers.clear();
        waitSemaphores.clear();
        signalSemaphores.clear();
        signalFence = nullptr;
    }
};

struct PresentInfo
{
    std::vector<SwapChain*> swapChains;
    std::vector<Semaphore*> waitSemaphores;
};

}