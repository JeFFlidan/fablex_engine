#pragma once

#include "internal.h"

namespace fe::rhi
{

struct Buffer;

struct BufferViewInfo
{
    const Buffer* buffer = nullptr;

    uint64 offset = 0;
    uint64 size = 0;
    ViewType type = ViewType::AUTO;
    Format newFormat = Format::UNDEFINED; 
};

struct BufferView
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkBufferView bufferView = VK_NULL_HANDLE;
#endif // FE_VULKAN
    };

    struct D3D12
    {
#if defined(FE_D3D12)
        D3D12_CPU_DESCRIPTOR_HANDLE handle;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()

    const Buffer* buffer = nullptr;

    uint64 descriptorIndex : 32;
    uint64 offset : 32;
    uint64 size : 32;
    ViewType type;
    Format format;
};

}