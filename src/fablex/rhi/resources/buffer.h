#pragma once

#include "internal.h"

namespace fe::rhi
{

struct BufferInfo
{
    uint64 size = 0;
    ResourceUsage bufferUsage = ResourceUsage::UNDEFINED;
    MemoryUsage memoryUsage = MemoryUsage::AUTO;
    ResourceFlags flags = ResourceFlags::UNDEFINED;
    Format format = Format::UNDEFINED;

    // Init data can be used only with memory usage CPU_ONLY and GPU_TO_CPU
    void* initData = nullptr;
    uint64 initDataSize;
};

struct alignas(64) Buffer
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation_T* allocation;
        VkDeviceAddress address = 0;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()

    uint64 size : 32;
    uint64 descriptorIndex : 32;
    
    ResourceUsage bufferUsage;
    MemoryUsage memoryUsage : 3;
    ResourceFlags flags : 20;
    Format format : 8;

    void* mappedData = nullptr;
};

FE_COMPILE_CHECK(sizeof(Buffer) == sizeof(uint64) * 8);

}