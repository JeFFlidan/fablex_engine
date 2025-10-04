#pragma once

#include "internal.h"

namespace fe::rhi
{

struct SwapChainInfo
{
    bool vSync = true;
    uint32 bufferCount = 3;
    Window* window = nullptr;
    Format format = Format::B8G8R8A8_UNORM;
    ColorSpace colorSpace = ColorSpace::SRGB;
    bool useHDR = false;
};

struct SwapChain
{
    struct Vulkan
    {
#if defined(FE_VULKAN) 
        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        uint32 imageIndex;
#endif // FE_VULKAN
    };

    FE_DEFINE_RHI_RESOURCE()

    Window* window = nullptr;
    ColorSpace colorSpace;
    Format format = Format::UNDEFINED;
    uint32 bufferCount = 0;
    bool vSync = true;
};

}