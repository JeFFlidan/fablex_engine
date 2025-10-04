#pragma once

#include "internal.h"

namespace fe::rhi
{

struct SamplerInfo
{
    Filter filter = Filter::UNDEFINED;
    AddressMode addressMode = AddressMode::UNDEFINED;
    BorderColor borderColor = BorderColor::UNDEFINED;
    float minLod = 0.0f;
    float maxLod = 1.0f;
    float maxAnisotropy = 1.0f;
};

struct Sampler
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkSampler sampler = VK_NULL_HANDLE;        
#endif // FE_VULKAN
    };

    FE_DEFINE_RHI_RESOURCE()

    uint32 descriptorIndex;
};

}