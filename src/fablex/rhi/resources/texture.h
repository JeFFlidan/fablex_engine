#pragma once

#include "internal.h"

namespace fe::rhi
{

struct TextureInfo
{
    uint32 width = 0;
    uint32 height = 0;
    // Only for 3D textures
    uint32 depth = 1;
    uint32 mipLevels = 1;
    uint32 layersCount = 1;
    Format format = Format::UNDEFINED;
    ResourceUsage textureUsage = ResourceUsage::UNDEFINED;
    MemoryUsage memoryUsage = MemoryUsage::AUTO;
    SampleCount samplesCount = SampleCount::UNDEFINED;
    TextureDimension dimension = TextureDimension::UNDEFINED;
    ResourceFlags flags = ResourceFlags::UNDEFINED;	// not necessary
    ComponentMapping componentMapping = { ComponentSwizzle::R, ComponentSwizzle::G, ComponentSwizzle::B, ComponentSwizzle::A };
};

struct alignas(64) Texture
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation_T* allocation;
#endif // FE_VULKAN
    };

    FE_DEFINE_RHI_RESOURCE()

    uint32 width : 16;
    uint32 height : 16;
    uint32 depth : 16;
    uint32 mipLevels : 6;
    uint32 layersCount : 10;

    Format format : 8;
    ResourceUsage textureUsage : 24;
    MemoryUsage memoryUsage : 3;
    TextureDimension dimension : 3;
    ResourceFlags flags : 20;

    void* mappedData;
};

FE_COMPILE_CHECK(sizeof(Texture) == sizeof(uint64) * 8);

}