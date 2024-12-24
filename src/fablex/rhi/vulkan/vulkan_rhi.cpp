#include "vulkan_rhi.h"
#include "vulkan_api.h"
#include "rhi/rhi.h"
#include "rhi/resources.h"
#include "core/logger.h"

#ifndef VOLK_IMPLEMENTATION
    #define VOLK_IMPLEMENTATION
#endif

#include "volk.h"

#ifndef VMA_IMPLEMENTATION
    #define VMA_IMPLEMENTATION
#endif
#include "vk_mem_alloc.h"

#include <iostream>

#define VK_CHECK(x)                                                     \
    do                                                                  \
    {                                                                   \
        VkResult err = x;                                               \
        if (err)                                                        \
        {                                                               \
            std::cout << "Detected Vulkan error " << err << std::endl;  \
            abort();                                                    \
        }                                                               \
    } while (0)

DEFINE_LOG_CATEGORY(LogVulkanRHI)

namespace fe::rhi::vulkan
{

VkFormat get_format(rhi::Format format)
{
    switch (format)
    {
        case rhi::Format::UNDEFINED:
            return VK_FORMAT_UNDEFINED;
        case rhi::Format::R4G4_UNORM:
            return VK_FORMAT_R4G4_UNORM_PACK8;
        case rhi::Format::R4G4B4A4_UNORM:
            return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        case rhi::Format::B4G4R4A4_UNORM:
            return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
        case rhi::Format::R5G5B5A1_UNORM:
            return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
        case rhi::Format::B5G5R5A1_UNORM:
            return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
        case rhi::Format::A1R5G5B5_UNORM:
            return VK_FORMAT_A1R5G5B5_UNORM_PACK16;

        case rhi::Format::R8_UNORM:
            return VK_FORMAT_R8_UNORM;
        case rhi::Format::R8_SNORM:
            return VK_FORMAT_R8_SNORM;
        case rhi::Format::R8_UINT:
            return VK_FORMAT_R8_UINT;
        case rhi::Format::R8_SINT:
            return VK_FORMAT_R8_SINT;
        case rhi::Format::R8_SRGB:
            return VK_FORMAT_R8_SRGB;

        case rhi::Format::R8G8_UNORM:
            return VK_FORMAT_R8G8_UNORM;
        case rhi::Format::R8G8_SNORM:
            return VK_FORMAT_R8G8_SNORM;
        case rhi::Format::R8G8_UINT:
            return VK_FORMAT_R8G8_UINT;
        case rhi::Format::R8G8_SINT:
            return VK_FORMAT_R8G8_SINT;
        case rhi::Format::R8G8_SRGB:
            return VK_FORMAT_R8G8_SRGB;

        case rhi::Format::R8G8B8A8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case rhi::Format::R8G8B8A8_SNORM:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case rhi::Format::R8G8B8A8_UINT:
            return VK_FORMAT_R8G8B8A8_UINT;
        case rhi::Format::R8G8B8A8_SINT:
            return VK_FORMAT_R8G8B8A8_SINT;
        case rhi::Format::R8G8B8A8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;

        case rhi::Format::B8G8R8A8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case rhi::Format::B8G8R8A8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case rhi::Format::B8G8R8A8_SNORM:
            return VK_FORMAT_B8G8R8A8_SNORM;

        case rhi::Format::R10G10B10A2_UNORM:
            return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        case rhi::Format::R10G10B10A2_SNORM:
            return VK_FORMAT_A2R10G10B10_SNORM_PACK32;

        case rhi::Format::R16_UNORM:
            return VK_FORMAT_R16_UNORM;
        case rhi::Format::R16_SNORM:
            return VK_FORMAT_R16_SNORM;
        case rhi::Format::R16_UINT:
            return VK_FORMAT_R16_UINT;
        case rhi::Format::R16_SINT:
            return VK_FORMAT_R16_SINT;
        case rhi::Format::R16_SFLOAT:
            return VK_FORMAT_R16_SFLOAT;

        case rhi::Format::R16G16_UNORM:
            return VK_FORMAT_R16G16_UNORM;
        case rhi::Format::R16G16_SNORM:
            return VK_FORMAT_R16G16_SNORM;
        case rhi::Format::R16G16_UINT:
            return VK_FORMAT_R16G16_UINT;
        case rhi::Format::R16G16_SINT:
            return VK_FORMAT_R16G16_SINT;
        case rhi::Format::R16G16_SFLOAT:
            return VK_FORMAT_R16G16_SFLOAT;

        case rhi::Format::R16G16B16A16_UNORM:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case rhi::Format::R16G16B16A16_SNORM:
            return VK_FORMAT_R16G16B16A16_SNORM;
        case rhi::Format::R16G16B16A16_UINT:
            return VK_FORMAT_R16G16B16A16_UINT;
        case rhi::Format::R16G16B16A16_SINT:
            return VK_FORMAT_R16G16B16A16_SINT;
        case rhi::Format::R16G16B16A16_SFLOAT:
            return VK_FORMAT_R16G16B16A16_SFLOAT;

        case rhi::Format::R32_UINT:
            return VK_FORMAT_R32_UINT;
        case rhi::Format::R32_SINT:
            return VK_FORMAT_R32_SINT;
        case rhi::Format::R32_SFLOAT:
            return VK_FORMAT_R32_SFLOAT;
        case rhi::Format::R32G32_UINT:
            return VK_FORMAT_R32G32_UINT;
        case rhi::Format::R32G32_SINT:
            return VK_FORMAT_R32G32_SINT;
        case rhi::Format::R32G32_SFLOAT:
            return VK_FORMAT_R32G32_SFLOAT;

        case rhi::Format::R32G32B32_UINT:
            return VK_FORMAT_R32G32B32_UINT;
        case rhi::Format::R32G32B32_SINT:
            return VK_FORMAT_R32G32B32_SINT;
        case rhi::Format::R32G32B32_SFLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;

        case rhi::Format::R32G32B32A32_UINT:
            return VK_FORMAT_R32G32B32A32_UINT;
        case rhi::Format::R32G32B32A32_SINT:
            return VK_FORMAT_R32G32B32A32_SINT;
        case rhi::Format::R32G32B32A32_SFLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;

        case rhi::Format::D16_UNORM:
            return VK_FORMAT_D16_UNORM;
        case rhi::Format::D32_SFLOAT:
            return VK_FORMAT_D32_SFLOAT;

        case rhi::Format::S8_UINT:
            return VK_FORMAT_S8_UINT;
        case rhi::Format::D16_UNORM_S8_UINT:
            return VK_FORMAT_D16_UNORM_S8_UINT;
        case rhi::Format::D24_UNORM_S8_UINT:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case rhi::Format::D32_SFLOAT_S8_UINT:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
            
        case rhi::Format::BC1_RGBA_UNORM:
            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case rhi::Format::BC1_RGBA_SRGB_UNORM:
            return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case rhi::Format::BC2_UNORM:
            return VK_FORMAT_BC2_UNORM_BLOCK;
        case rhi::Format::BC2_SRGB:
            return VK_FORMAT_BC2_SRGB_BLOCK;
        case rhi::Format::BC3_UNORM:
            return VK_FORMAT_BC3_UNORM_BLOCK;
        case rhi::Format::BC3_SRGB:
            return VK_FORMAT_BC3_SRGB_BLOCK;
        case rhi::Format::BC4_UNORM:
            return VK_FORMAT_BC4_UNORM_BLOCK;
        case rhi::Format::BC4_SNORM:
            return VK_FORMAT_BC4_SNORM_BLOCK;
        case rhi::Format::BC5_UNORM:
            return VK_FORMAT_BC5_UNORM_BLOCK;
        case rhi::Format::BC5_SNORM:
            return VK_FORMAT_BC5_SNORM_BLOCK;
        case rhi::Format::BC6H_UFLOAT:
            return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case rhi::Format::BC6H_SFLOAT:
            return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case rhi::Format::BC7_UNORM:
            return VK_FORMAT_BC7_UNORM_BLOCK;
        case rhi::Format::BC7_SRGB:
            return VK_FORMAT_BC7_SRGB_BLOCK;
    }

    return VK_FORMAT_UNDEFINED;
}

VkSampleCountFlagBits get_sample_count(rhi::SampleCount sampleCount)
{
    switch (sampleCount)
    {
        default:
        case rhi::SampleCount::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Sample count is UNDEFINED.");
            return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
        case rhi::SampleCount::BIT_1:
            return VK_SAMPLE_COUNT_1_BIT;
        case rhi::SampleCount::BIT_2:
            return VK_SAMPLE_COUNT_2_BIT;
        case rhi::SampleCount::BIT_4:
            return VK_SAMPLE_COUNT_4_BIT;
        case rhi::SampleCount::BIT_8:
            return VK_SAMPLE_COUNT_8_BIT;
        case rhi::SampleCount::BIT_16:
            return VK_SAMPLE_COUNT_16_BIT;
        case rhi::SampleCount::BIT_32:
            return VK_SAMPLE_COUNT_32_BIT;
        case rhi::SampleCount::BIT_64:
            return VK_SAMPLE_COUNT_64_BIT;
    }
}

VmaMemoryUsage get_memory_usage(rhi::MemoryUsage memoryUsage)
{
    switch (memoryUsage)
    {
        default:
        case rhi::MemoryUsage::AUTO:
        case rhi::MemoryUsage::CPU_TO_GPU:
        case rhi::MemoryUsage::GPU_TO_CPU:
            return VMA_MEMORY_USAGE_AUTO;
        case rhi::MemoryUsage::CPU:
            return VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        case rhi::MemoryUsage::GPU:
            return VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    }
}

VkImageType get_image_type(rhi::TextureDimension dimension)
{
    switch (dimension)
    {
        default:
        case rhi::TextureDimension::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Texture dimension is UNDEFINED.");
            return VK_IMAGE_TYPE_MAX_ENUM;
        case rhi::TextureDimension::TEXTURE1D:
            return VK_IMAGE_TYPE_1D;
        case rhi::TextureDimension::TEXTURE2D:
            return VK_IMAGE_TYPE_2D;
        case rhi::TextureDimension::TEXTURE3D:
            return VK_IMAGE_TYPE_3D;
    }
}

void get_filter(rhi::Filter filter, VkSamplerCreateInfo& samplerInfo)
{
    switch (filter)
    {
        case rhi::Filter::MIN_MAG_MIP_NEAREST:
        case rhi::Filter::MINIMUM_MIN_MAG_MIP_NEAREST:
        case rhi::Filter::MAXIMUM_MIN_MAG_MIP_NEAREST:
        case rhi::Filter::COMPARISON_MIN_MAG_MIP_NEAREST:
            samplerInfo.minFilter = VK_FILTER_NEAREST;
            samplerInfo.magFilter = VK_FILTER_NEAREST;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case rhi::Filter::MIN_MAG_NEAREST_MIP_LINEAR:
        case rhi::Filter::MINIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
        case rhi::Filter::MAXIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
        case rhi::Filter::COMPARISON_MIN_MAG_NEAREST_MIP_LINEAR:
            samplerInfo.minFilter = VK_FILTER_NEAREST;
            samplerInfo.magFilter = VK_FILTER_NEAREST;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case rhi::Filter::MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
        case rhi::Filter::MINIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
        case rhi::Filter::MAXIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
        case rhi::Filter::COMPARISON_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
            samplerInfo.minFilter = VK_FILTER_NEAREST;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case rhi::Filter::MIN_NEAREST_MAG_MIP_LINEAR:
        case rhi::Filter::MINIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
        case rhi::Filter::MAXIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
        case rhi::Filter::COMPARISON_MIN_NEAREST_MAG_MIP_LINEAR:
            samplerInfo.minFilter = VK_FILTER_NEAREST;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case rhi::Filter::MIN_LINEAR_MAG_MIP_NEAREST:
        case rhi::Filter::MINIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
        case rhi::Filter::MAXIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
        case rhi::Filter::COMPARISON_MIN_LINEAR_MAG_MIP_NEAREST:
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.magFilter = VK_FILTER_NEAREST;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case rhi::Filter::MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
        case rhi::Filter::MINIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
        case rhi::Filter::MAXIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
        case rhi::Filter::COMPARISON_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.magFilter = VK_FILTER_NEAREST;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case rhi::Filter::MIN_MAG_LINEAR_MIP_NEAREST:
        case rhi::Filter::MINIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
        case rhi::Filter::MAXIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
        case rhi::Filter::COMPARISON_MIN_MAG_LINEAR_MIP_NEAREST:
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case rhi::Filter::MIN_MAG_MIP_LINEAR:
        case rhi::Filter::MINIMUM_MIN_MAG_MIP_LINEAR:
        case rhi::Filter::MAXIMUM_MIN_MAG_MIP_LINEAR:
        case rhi::Filter::COMPARISON_MIN_MAG_MIP_LINEAR:
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case rhi::Filter::ANISOTROPIC:
        case rhi::Filter::MINIMUM_ANISOTROPIC:
        case rhi::Filter::MAXIMUM_ANISOTROPIC:
        case rhi::Filter::COMPARISON_ANISOTROPIC:
            // have to think about this
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.anisotropyEnable = VK_TRUE;
            break;
        default:
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
    }
}

VkBorderColor get_border_color(rhi::BorderColor borderColor)
{
    switch (borderColor)
    {
        default:
        case rhi::BorderColor::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Border color is UNDEFINED");
            return VK_BORDER_COLOR_MAX_ENUM;
        case rhi::BorderColor::FLOAT_TRANSPARENT_BLACK:
            return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        case rhi::BorderColor::INT_TRANSPARENT_BLACK:
            return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
        case rhi::BorderColor::FLOAT_OPAQUE_BLACK:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        case rhi::BorderColor::INT_OPAQUE_BLACK:
            return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        case rhi::BorderColor::FLOAT_OPAQUE_WHITE:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        case rhi::BorderColor::INT_OPAQUE_WHITE:
            return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    }
}

VkSamplerAddressMode get_address_mode(rhi::AddressMode addressMode)
{
    switch (addressMode)
    {
        default:
        case rhi::AddressMode::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Address mode is UNDEFINED.");
            return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
        case rhi::AddressMode::REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case rhi::AddressMode::MIRRORED_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case rhi::AddressMode::CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case rhi::AddressMode::CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case rhi::AddressMode::MIRROR_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    }
}

VkBufferUsageFlags get_buffer_usage(rhi::ResourceUsage usage)
{
    VkBufferUsageFlags usageFlags = 0;
    if (has_flag(usage, rhi::ResourceUsage::TRANSFER_SRC))
    {
        usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::TRANSFER_DST))
    {
        usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::UNIFORM_TEXEL_BUFFER))
    {
        usageFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::STORAGE_TEXEL_BUFFER))
    {
        usageFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::UNIFORM_BUFFER))
    {
        usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::STORAGE_BUFFER))
    {
        usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::INDEX_BUFFER))
    {
        usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::VERTEX_BUFFER))
    {
        usageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::INDIRECT_BUFFER))
    {
        usageFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    return usageFlags;
}

VkImageUsageFlags get_image_usage(rhi::ResourceUsage usage)
{
    VkImageUsageFlags usageFlags = 0;
    if (has_flag(usage, rhi::ResourceUsage::TRANSFER_SRC))
    {
        usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::TRANSFER_DST))
    {
        usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::SAMPLED_TEXTURE))
    {
        usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::STORAGE_TEXTURE))
    {
        usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::COLOR_ATTACHMENT))
    {
        usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT))
    {
        usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::TRANSIENT_ATTACHMENT))
    {
        usageFlags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    }
    if (has_flag(usage, rhi::ResourceUsage::INPUT_ATTACHMENT))
    {
        usageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    return usageFlags;
}

VkPrimitiveTopology get_primitive_topology(rhi::TopologyType topologyType)
{
    switch (topologyType)
    {
        default:
        case rhi::TopologyType::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Topology type is UNDEFINED.");
            return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
        case rhi::TopologyType::POINT:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case rhi::TopologyType::LINE:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case rhi::TopologyType::TRIANGLE:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case rhi::TopologyType::PATCH:
            return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    }
}

VkPolygonMode get_polygon_mode(rhi::PolygonMode polygonMode)
{
    switch (polygonMode)
    {
        default:
        case rhi::PolygonMode::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Polygon mode is UNDEFINED.");
            return VK_POLYGON_MODE_MAX_ENUM;
        case rhi::PolygonMode::FILL:
            return VK_POLYGON_MODE_FILL;
        case rhi::PolygonMode::LINE:
            return VK_POLYGON_MODE_LINE;
        case rhi::PolygonMode::POINT:
            return VK_POLYGON_MODE_POINT;
    }
}

VkCullModeFlags get_cull_mode(rhi::CullMode cullMode)
{
    switch (cullMode)
    {
        default:
        case rhi::CullMode::UNDEFINED:
            return 0;
        case rhi::CullMode::NONE:
            return VK_CULL_MODE_NONE;
        case rhi::CullMode::FRONT:
            return VK_CULL_MODE_FRONT_BIT;
        case rhi::CullMode::BACK:
            return VK_CULL_MODE_BACK_BIT;
        case rhi::CullMode::FRONT_AND_BACK:
            return VK_CULL_MODE_FRONT_AND_BACK;
    }
}

VkFrontFace get_front_face(rhi::FrontFace frontFace)
{
    switch (frontFace)
    {
        default:
        case rhi::FrontFace::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Front face is UNDEFINED.");
            return VK_FRONT_FACE_MAX_ENUM;
        case rhi::FrontFace::CLOCKWISE:
            return VK_FRONT_FACE_CLOCKWISE;
        case rhi::FrontFace::COUNTER_CLOCKWISE:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
}

VkShaderStageFlags get_shader_stage(rhi::ShaderType shaderType)
{
    switch (shaderType)
    {
        default:
        case rhi::ShaderType::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Shader type is UNDEFINED.");
            return 0;
        case rhi::ShaderType::VERTEX:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case rhi::ShaderType::FRAGMENT:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case rhi::ShaderType::TESSELLATION_CONTROL:
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case rhi::ShaderType::TESSELLATION_EVALUATION:
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case rhi::ShaderType::GEOMETRY:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case rhi::ShaderType::COMPUTE:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        case rhi::ShaderType::MESH:
            return VK_SHADER_STAGE_MESH_BIT_NV;
        case rhi::ShaderType::TASK:
            return VK_SHADER_STAGE_TASK_BIT_NV;
        case rhi::ShaderType::RAY_GENERATION:
            return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        case rhi::ShaderType::RAY_INTERSECTION:
            return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        case rhi::ShaderType::RAY_ANY_HIT:
            return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        case rhi::ShaderType::RAY_CLOSEST_HIT:
            return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        case rhi::ShaderType::RAY_MISS:
            return VK_SHADER_STAGE_MISS_BIT_KHR;
        case rhi::ShaderType::RAY_CALLABLE:
            return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
    }
}

VkLogicOp get_logic_op(rhi::LogicOp logicOp)
{
    switch (logicOp)
    {
        default:
        case rhi::LogicOp::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Logic op is UNDEFINED.");
            return VK_LOGIC_OP_MAX_ENUM;
        case rhi::LogicOp::CLEAR:
            return VK_LOGIC_OP_CLEAR;
        case rhi::LogicOp::AND:
            return VK_LOGIC_OP_AND;
        case rhi::LogicOp::AND_REVERSE:
            return VK_LOGIC_OP_AND_REVERSE;
        case rhi::LogicOp::COPY:
            return VK_LOGIC_OP_COPY;
        case rhi::LogicOp::AND_INVERTED:
            return VK_LOGIC_OP_AND_INVERTED;
        case rhi::LogicOp::NO_OP:
            return VK_LOGIC_OP_NO_OP;
        case rhi::LogicOp::XOR:
            return VK_LOGIC_OP_XOR;
        case rhi::LogicOp::OR:
            return VK_LOGIC_OP_OR;
        case rhi::LogicOp::NOR:
            return VK_LOGIC_OP_NOR;
        case rhi::LogicOp::EQUIVALENT:
            return VK_LOGIC_OP_EQUIVALENT;
        case rhi::LogicOp::INVERT:
            return VK_LOGIC_OP_INVERT;
        case rhi::LogicOp::OR_REVERSE:
            return VK_LOGIC_OP_OR_REVERSE;
        case rhi::LogicOp::COPY_INVERTED:
            return VK_LOGIC_OP_COPY_INVERTED;
        case rhi::LogicOp::OR_INVERTED:
            return VK_LOGIC_OP_OR_INVERTED;
        case rhi::LogicOp::NAND:
            return VK_LOGIC_OP_NAND;
        case rhi::LogicOp::SET:
            return VK_LOGIC_OP_SET;
    }
}

VkBlendFactor get_blend_factor(rhi::BlendFactor blendFactor)
{
    switch (blendFactor)
    {
        default:
        case rhi::BlendFactor::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Blend factor is UNDEFINED.");
            return VK_BLEND_FACTOR_MAX_ENUM;
        case rhi::BlendFactor::ZERO:
            return VK_BLEND_FACTOR_ZERO;
        case rhi::BlendFactor::ONE:
            return VK_BLEND_FACTOR_ONE;
        case rhi::BlendFactor::SRC_COLOR:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case rhi::BlendFactor::ONE_MINUS_SRC_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case rhi::BlendFactor::DST_COLOR:
            return VK_BLEND_FACTOR_DST_COLOR;
        case rhi::BlendFactor::ONE_MINUS_DST_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case rhi::BlendFactor::SRC_ALPHA:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case rhi::BlendFactor::ONE_MINUS_SRC_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case rhi::BlendFactor::DST_ALPHA:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case rhi::BlendFactor::ONE_MINUS_DST_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case rhi::BlendFactor::CONSTANT_COLOR:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case rhi::BlendFactor::ONE_MINUS_CONSTANT_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case rhi::BlendFactor::CONSTANT_ALPHA:
            return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case rhi::BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        case rhi::BlendFactor::SRC_ALPHA_SATURATE:
            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case rhi::BlendFactor::SRC1_COLOR:
            return VK_BLEND_FACTOR_SRC1_COLOR;
        case rhi::BlendFactor::ONE_MINUS_SRC1_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case rhi::BlendFactor::SRC1_ALPHA:
            return VK_BLEND_FACTOR_SRC1_ALPHA;
        case rhi::BlendFactor::ONE_MINUS_SRC1_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    }
}

VkBlendOp get_blend_op(rhi::BlendOp blendOp)
{
    switch (blendOp)
    {
        default:
        case rhi::BlendOp::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Blend op is UNDEFINED.");
            return VK_BLEND_OP_MAX_ENUM;
        case rhi::BlendOp::ADD:
            return VK_BLEND_OP_ADD;
        case rhi::BlendOp::SUBTRACT:
            return VK_BLEND_OP_SUBTRACT;
        case rhi::BlendOp::REVERSE_SUBTRACT:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case rhi::BlendOp::MIN:
            return VK_BLEND_OP_MIN;
        case rhi::BlendOp::MAX:
            return VK_BLEND_OP_MAX;
    }
}

VkCompareOp get_compare_op(rhi::CompareOp compareOp)
{
    switch (compareOp)
    {
        default:
        case rhi::CompareOp::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Compare op is UNDEFINED.");
            return VK_COMPARE_OP_MAX_ENUM;
        case rhi::CompareOp::NEVER:
            return VK_COMPARE_OP_NEVER;
        case rhi::CompareOp::LESS:
            return VK_COMPARE_OP_LESS;
        case rhi::CompareOp::EQUAL:
            return VK_COMPARE_OP_EQUAL;
        case rhi::CompareOp::LESS_OR_EQUAL:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case rhi::CompareOp::GREATER:
            return VK_COMPARE_OP_GREATER;
        case rhi::CompareOp::NOT_EQUAL:
            return VK_COMPARE_OP_NOT_EQUAL;
        case rhi::CompareOp::GREATER_OR_EQUAL:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case rhi::CompareOp::ALWAYS:
            return VK_COMPARE_OP_ALWAYS;
    }
}

VkStencilOp get_stencil_op(rhi::StencilOp stencilOp)
{
    switch (stencilOp)
    {
        default:
        case rhi::StencilOp::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Stencil op is UNDEFINED.");
            return VK_STENCIL_OP_MAX_ENUM;
        case rhi::StencilOp::KEEP:
            return VK_STENCIL_OP_KEEP;
        case rhi::StencilOp::ZERO:
            return VK_STENCIL_OP_ZERO;
        case rhi::StencilOp::REPLACE:
            return VK_STENCIL_OP_REPLACE;
        case rhi::StencilOp::INCREMENT_AND_CLAMP:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case rhi::StencilOp::DECREMENT_AND_CLAMP:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case rhi::StencilOp::INVERT:
            return VK_STENCIL_OP_INVERT;
        case rhi::StencilOp::INCREMENT_AND_WRAP:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case rhi::StencilOp::DECREMENT_AND_WRAP:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    }
}

VkAttachmentLoadOp get_attach_load_op(rhi::LoadOp loadOp)
{
    switch (loadOp)
    {
        default:
        case rhi::LoadOp::LOAD:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case rhi::LoadOp::CLEAR:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case rhi::LoadOp::DONT_CARE:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
}

VkAttachmentStoreOp get_attach_store_op(rhi::StoreOp storeOp)
{
    switch (storeOp)
    {
        default:
        case rhi::StoreOp::STORE:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case rhi::StoreOp::DONT_CARE:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
}

VkImageLayout get_image_layout(rhi::ResourceLayout resourceLayout)
{
    if (has_flag(resourceLayout, rhi::ResourceLayout::UNDEFINED))
    {
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::GENERAL))
    {
        return VK_IMAGE_LAYOUT_GENERAL;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::SHADER_READ))
    {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::COLOR_ATTACHMENT))
    {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::DEPTH_STENCIL))
    {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::DEPTH_STENCIL_READ_ONLY))
    {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::TRANSFER_SRC))
    {
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::TRANSFER_DST))
    {
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::PRESENT_ATTACHMENT))
    {
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkAccessFlags2 get_access(rhi::ResourceLayout resourceLayout)
{
    VkAccessFlags accessFlags = 0;
    if (has_flag(resourceLayout, rhi::ResourceLayout::INDIRECT_COMMAND_BUFFER))
    {
        accessFlags |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::INDEX_BUFFER))
    {
        accessFlags |= VK_ACCESS_2_INDEX_READ_BIT;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::VERTEX_BUFFER))
    {
        accessFlags |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::UNIFORM_BUFFER))
    {
        accessFlags |= VK_ACCESS_2_UNIFORM_READ_BIT;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::SHADER_READ))
    {
        accessFlags |= VK_ACCESS_2_SHADER_READ_BIT;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::SHADER_WRITE))
    {
        accessFlags |= VK_ACCESS_2_SHADER_WRITE_BIT;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::COLOR_ATTACHMENT))
    {
        accessFlags |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::DEPTH_STENCIL))
    {
        accessFlags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::DEPTH_STENCIL_READ_ONLY))
    {
        accessFlags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::TRANSFER_SRC))
    {
        accessFlags |= VK_ACCESS_2_TRANSFER_READ_BIT;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::TRANSFER_DST))
    {
        accessFlags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::MEMORY_READ))
    {
        accessFlags |= VK_ACCESS_2_MEMORY_READ_BIT;
    }
    if (has_flag(resourceLayout, rhi::ResourceLayout::MEMORY_WRITE))
    {
        accessFlags |= VK_ACCESS_2_MEMORY_WRITE_BIT;
    }
    return accessFlags;
}

VkPipelineBindPoint get_pipeline_bind_point(rhi::PipelineType pipelineType)
{
    switch (pipelineType)
    {
        default:
        case rhi::PipelineType::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Pipeline type is UNDEFINED.");
            return VK_PIPELINE_BIND_POINT_MAX_ENUM;
        case rhi::PipelineType::GRAPHICS:
            return VK_PIPELINE_BIND_POINT_GRAPHICS;
        case rhi::PipelineType::COMPUTE:
            return VK_PIPELINE_BIND_POINT_COMPUTE;
        case rhi::PipelineType::RAY_TRACING:
            return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
    }
}

VkImageAspectFlags get_image_aspect(rhi::ResourceUsage usage)
{
    if (has_flag(usage, rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT))
    {
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    return VK_IMAGE_ASPECT_COLOR_BIT;
}

VkImageAspectFlags get_image_aspect(rhi::TextureAspect textureAspect)
{
    switch (textureAspect)
    {
        default:
        case rhi::TextureAspect::UNDEFINED:
            FE_LOG(LogVulkanRHI, ERROR, "Texture aspect is UNDEFINED.");
            return VK_IMAGE_ASPECT_NONE;
        case rhi::TextureAspect::COLOR:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        case rhi::TextureAspect::DEPTH:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case rhi::TextureAspect::STENCIL:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
    }
}

// VkQueryType get_query_type(rhi::QueryType queryType)
// {
//     switch (queryType)
//     {
//         case rhi::QueryType::OCCLUSION:
//         case rhi::QueryType::BINARY_OCCLUSION:
//             return VK_QUERY_TYPE_OCCLUSION;
//         case rhi::QueryType::PIPELINE_STATISTICS:
//             return VK_QUERY_TYPE_PIPELINE_STATISTICS;
//         case rhi::QueryType::TIMESTAMP:
//             return VK_QUERY_TYPE_TIMESTAMP;
//     }
// }

VkComponentSwizzle get_component_swizzle(rhi::ComponentSwizzle swizzle)
{
    switch (swizzle)
    {
        case rhi::ComponentSwizzle::R:
            return VK_COMPONENT_SWIZZLE_R;
        case rhi::ComponentSwizzle::G:
            return VK_COMPONENT_SWIZZLE_G;
        case rhi::ComponentSwizzle::B:
            return VK_COMPONENT_SWIZZLE_B;
        case rhi::ComponentSwizzle::A:
            return VK_COMPONENT_SWIZZLE_A;
        case rhi::ComponentSwizzle::ONE:
            return VK_COMPONENT_SWIZZLE_ONE;
        case rhi::ComponentSwizzle::ZERO:
            return VK_COMPONENT_SWIZZLE_ZERO;
        default:
            FE_LOG(LogVulkanRHI, ERROR, "Swizzle is UNDEFINED.");
            return VK_COMPONENT_SWIZZLE_IDENTITY;
    }
}

VkComponentMapping get_component_mapping(const rhi::ComponentMapping& mapping)
{
    VkComponentMapping vkMapping;
    vkMapping.r = get_component_swizzle(mapping.r);
    vkMapping.g = get_component_swizzle(mapping.g);
    vkMapping.b = get_component_swizzle(mapping.b);
    vkMapping.a = get_component_swizzle(mapping.a);
    return vkMapping;
}

void init(const RHIInitInfo* initInfo)
{
    std::cout << "Vulkan is initialized" << std::endl;
}

void cleanup()
{

}

void create_swap_chain(SwapChain** swapChain, const SwapChainInfo* info)
{

}

void destroy_swap_chain(SwapChain* swapChain)
{

}

void create_buffer(Buffer** buffer, const BufferInfo* info)
{

}

void update_buffer(Buffer*, uint64 size, const void* data)
{

}

void destroy_buffer(Buffer* buffer)
{

}

void create_texture(Texture** texture, const TextureInfo* info)
{

}

void destroy_texture(Texture* texture)
{

}

void create_texture_view(TextureView** textureView, const TextureViewInfo* info, const Texture* texture)
{

}

void destroy_texture_view(TextureView* textureView)
{

}

void create_buffer_view(BufferView** bufferView, const BufferViewInfo* info)
{

}

void destroy_buffer_view(BufferView* bufferView)
{

}

void create_sampler(Sampler** sampler, const SamplerInfo* info)
{

}

void destroy_sampler(Sampler* sampler)
{

}

void create_shader(Shader** shader, const ShaderInfo* info)
{

}

void destroy_shader(Shader* shader)
{

}

void create_graphics_pipelines(const std::vector<GraphicsPipelineInfo>& infos, std::vector<Pipeline*>& outPipelines)
{

}

void create_compute_pipelines(const std::vector<ComputePipelineInfo>& infos, std::vector<Pipeline*>& outPipelines)
{

}

void destroy_pipeline(Pipeline* pipeline)
{

}

uint32 get_buffer_index(const Buffer* buffer)
{
    return 0;
}

uint32 get_texture_view_index(const TextureView* textureView)
{
    return 0;
}

uint32 get_buffer_view_index(const BufferView* bufferView)
{
    return 0;
}

uint32 get_sampler_index(const Sampler* sampler)
{
    return 0;
}

void bind_uniform_buffer(Buffer* buffer, uint32 slot, uint32 size, uint32 offset)
{
    
}

void create_command_pool(CommandPool** cmdPool, const CommandPoolInfo* info)
{

}

void destroy_command_pool(CommandPool* cmdPool)
{

}

void create_command_buffer(CommandBuffer** cmd, const CommandBufferInfo* info)
{

}

void destroy_command_buffer(CommandBuffer* cmd)
{

}

void begin_command_buffer(CommandBuffer* cmd)
{

}

void end_command_buffer(CommandBuffer* cmd)
{

}

void wait_command_buffer(CommandBuffer* cmd1, CommandBuffer* cmd2)
{

}

void reset_command_pool(CommandPool* cmdPool)
{

}

void create_semaphore(Semaphore** semaphore)
{

}

void destroy_semaphore(Semaphore* semaphore)
{

}

void create_fence(Fence** fence)
{

}

void destroy_fence(Fence* fence)
{

}

void fill_buffer(CommandBuffer* cmd, Buffer* dstBuffer, uint32 dstOffset, uint32 size, uint32 data)
{

}

void copy_buffer(CommandBuffer* cmd, Buffer* srcBuffer, Buffer* dstBuffer, uint32 size, uint32 srcOffset, uint32 dstOffset)
{

}

void copy_texture(CommandBuffer* cmd, Texture* srcTexture, Texture* dstTexture)
{

}

void copy_buffer_to_texture(CommandBuffer* cmd, Buffer* srcBuffer, Texture* dstTexture)
{

}

void copy_texture_to_buffer(CommandBuffer* cmd, Texture* srcTexture, Buffer* dstBuffer)
{

}

void blit_texture(
    CommandBuffer* cmd, 
    Texture* srcTexture, 
    Texture* dstTexture, 
    std::array<int32, 3> srcOffset, 
    std::array<int32, 3> dstOffset, 
    uint32 srcMipLevel, 
    uint32 dstMipLevel, 
    uint32 srcBaseLayer, 
    uint32 dstBaseLayer
)
{

}

void set_viewports(CommandBuffer* cmd, const std::vector<Viewport>& viewports)
{

}

void set_scissors(CommandBuffer* cmd, const std::vector<Scissor>& scissors)
{

}

void push_constants(CommandBuffer* cmd, Pipeline* pipeline, void* data)
{

}

void bind_vertex_buffer(CommandBuffer* cmd, Buffer* buffer)
{

}

void bind_index_buffer(CommandBuffer* cmd, Buffer* buffer)
{

}

void bind_pipeline(CommandBuffer* cmd, Buffer* buffer)
{

}

void begin_rendering(CommandBuffer* cmd, RenderingBeginInfo* beginInfo)
{

}

void end_rendering(CommandBuffer* cmd)
{

}

void draw(CommandBuffer* cmd, uint64 vertexCount)
{

}

void draw_indirect(CommandBuffer* cmd, Buffer* buffer, uint32 offset, uint32 drawCount, uint32 stride)
{

}

void draw_indexed_indirect(CommandBuffer* cmd, Buffer* buffer, uint32 offset, uint32 drawCount, uint32 stride)
{

}

void dispatch(CommandBuffer* cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{

}

void add_pipeline_barriers(CommandBuffer* cmd, uint32 barrierCount, const PipelineBarrier* barriers)
{

}

void acquire_next_image(SwapChain* swapChain, Semaphore* signalSemaphore, Fence* fence, uint32* imageIndex)
{

}

void submit(CommandBuffer* cmd, SubmitInfo* submitInfo)
{

}

void present(PresentInfo* presentInfo)
{

}

void wait_queue_idle(QueueType queueType)
{

}

void wait_for_fences(const std::vector<Fence*>& fences)
{

}

void fill_function_table()
{
    fe::rhi::init = init;
    fe::rhi::cleanup = cleanup;

    fe::rhi::create_swap_chain = create_swap_chain;
    fe::rhi::destroy_swap_chain = destroy_swap_chain;

    fe::rhi::create_buffer = create_buffer;
    fe::rhi::update_buffer = update_buffer;
    fe::rhi::destroy_buffer = destroy_buffer;

    fe::rhi::create_texture = create_texture;
    fe::rhi::destroy_texture = destroy_texture;
    fe::rhi::create_texture_view = create_texture_view;
    fe::rhi::destroy_texture_view = destroy_texture_view;
    fe::rhi::create_buffer_view = create_buffer_view;
    fe::rhi::destroy_buffer_view = destroy_buffer_view;
    fe::rhi::create_sampler = create_sampler;
    fe::rhi::destroy_sampler = destroy_sampler;
    fe::rhi::create_shader = create_shader;
    fe::rhi::destroy_shader = destroy_shader;
    fe::rhi::create_graphics_pipelines = create_graphics_pipelines;
    fe::rhi::create_compute_pipelines = create_compute_pipelines;
    fe::rhi::destroy_pipeline = destroy_pipeline;

    fe::rhi::get_buffer_index = get_buffer_index;
    fe::rhi::get_texture_view_index = get_texture_view_index;
    fe::rhi::get_buffer_view_index = get_buffer_view_index;
    fe::rhi::get_sampler_index = get_sampler_index;

    fe::rhi::bind_uniform_buffer = bind_uniform_buffer;

    fe::rhi::create_command_pool = create_command_pool;
    fe::rhi::destroy_command_pool = destroy_command_pool;
    fe::rhi::create_command_buffer = create_command_buffer;
    fe::rhi::destroy_command_buffer = destroy_command_buffer;
    fe::rhi::begin_command_buffer = begin_command_buffer;
    fe::rhi::end_command_buffer = end_command_buffer;
    fe::rhi::wait_command_buffer = wait_command_buffer;
    fe::rhi::reset_command_pool = reset_command_pool;

    fe::rhi::create_semaphore = create_semaphore;
    fe::rhi::destroy_semaphore = destroy_semaphore;
    fe::rhi::create_fence = create_fence;
    fe::rhi::destroy_fence = destroy_fence;

    fe::rhi::fill_buffer = fill_buffer;
    fe::rhi::copy_buffer = copy_buffer;
    fe::rhi::copy_texture = copy_texture;
    fe::rhi::copy_buffer_to_texture = copy_buffer_to_texture;
    fe::rhi::copy_texture_to_buffer = copy_texture_to_buffer;
    fe::rhi::blit_texture = blit_texture;

    fe::rhi::set_viewports = set_viewports;
    fe::rhi::set_scissors = set_scissors;
    
    fe::rhi::push_constants = push_constants;
    fe::rhi::bind_vertex_buffer = bind_vertex_buffer;
    fe::rhi::bind_index_buffer = bind_index_buffer;
    fe::rhi::bind_pipeline = bind_pipeline;

    fe::rhi::begin_rendering = begin_rendering;
    fe::rhi::end_rendering = end_rendering;

    fe::rhi::draw = draw;
    fe::rhi::draw_indirect = draw_indirect;
    fe::rhi::draw_indexed_indirect = draw_indexed_indirect;

    fe::rhi::dispatch = dispatch;
    fe::rhi::add_pipeline_barriers = add_pipeline_barriers;

    fe::rhi::acquire_next_image = acquire_next_image;
    fe::rhi::submit = submit;
    fe::rhi::present = present;
    fe::rhi::wait_queue_idle = wait_queue_idle;
    fe::rhi::wait_for_fences = wait_for_fences;
}

}