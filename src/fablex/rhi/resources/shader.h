#pragma once

#include "internal.h"

namespace fe::rhi
{

struct ShaderInfo
{
    ShaderType shaderType = ShaderType::UNDEFINED;
    uint8_t* data = nullptr;		// Pointer to SPIRV or DXIL data (depends on chosen API)
    uint64_t size = 0;
};

struct Shader
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkShaderModule shader = VK_NULL_HANDLE;
#endif // FE_VULKAN
    };

    FE_DEFINE_RHI_RESOURCE()

    ShaderType type = ShaderType::UNDEFINED;
};

}