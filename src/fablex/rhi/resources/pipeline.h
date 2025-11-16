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

    struct D3D12
    {
#if defined(FE_D3D12)
        ID3D12PipelineState* pipelineState = nullptr;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()

    PipelineType type{PipelineType::UNDEFINED};
};

}