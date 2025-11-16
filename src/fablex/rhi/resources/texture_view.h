#pragma once

#include "internal.h"

namespace fe::rhi
{

struct Texture;
struct Buffer;

struct MipMap
{
    uint32 offset = 0;
    uint32 layer = 0;
};

struct TextureInitInfo
{
    Buffer* buffer;
    std::vector<MipMap> mipMaps;
};

struct TextureViewInfo
{
    const Texture* texture = nullptr;

    uint32 baseMipLevel = 0;
    uint32 baseLayer = 0;
    uint32 mipLevels = 1;
    uint32 layerCount = 1;
    // if texture aspect is undefined, rhi will automatically set aspect mask.
    // however, for stencil view it must be set
    TextureAspect aspect = TextureAspect::UNDEFINED;
    ViewType type = ViewType::AUTO;
    ComponentMapping componentMapping = { ComponentSwizzle::R, ComponentSwizzle::G, ComponentSwizzle::B, ComponentSwizzle::A };
    Format format = Format::UNDEFINED;	// If format is undefined, format from texture will be used
};

struct TextureView
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkImageView imageView = VK_NULL_HANDLE;
#endif // FE_VULKAN
    };

    struct D3D12
    {
#if defined(FE_D3D12)
        D3D12_CPU_DESCRIPTOR_HANDLE handle;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()

    const Texture* texture;

    uint32 descriptorIndex;

    uint32 baseMipLevel : 6;
    uint32 baseLayer : 10;
    uint32 mipLevels : 6;
    uint32 layerCount : 10;

    TextureAspect aspect : 3;
    ViewType type : 3;
    Format format : 8;
};

}