#pragma once

#include "internal.h"

namespace fe::rhi
{

struct Pipeline
{
    struct Vulkan
    {
#if defined (FE_VULKAN)
        VkPipeline pipeline = VK_NULL_HANDLE;
        uint64 layoutHash = 0;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()

    PipelineType type{PipelineType::UNDEFINED};
};

}