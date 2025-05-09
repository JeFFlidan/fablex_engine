#include "core/platform/platform.h"

#ifndef VK_NO_PROTOTYPES
	#define VK_NO_PROTOTYPES
#endif
#ifdef _WIN32
	#ifndef VK_USE_PLATFORM_WIN32_KHR
		#define VK_USE_PLATFORM_WIN32_KHR
	#endif
#endif

#include "vulkan_rhi.h"
#include "rhi/rhi.h"
#include "rhi/resources.h"
#include "rhi/utils.h"
#include "core/logger.h"
#include "core/pool_allocator.h"
#include "core/utils.h"

#define VOLK_IMPLEMENTATION
#include "volk.h"

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"

#include "spirv_reflect.h"

#include <unordered_map>

FE_DEFINE_LOG_CATEGORY(LogVulkanRHI)

#define VK_CHECK(x)                                                                 \
    do                                                                              \
    {                                                                               \
        VkResult err = x;                                                           \
        if (err)                                                                    \
        {                                                                           \
            FE_LOG(LogVulkanRHI, FATAL, "Detected Vulkan error {}", (int32)err);    \
        }                                                                           \
    } while (0)

#define CHECK_SHADER_REFLECTION(Result) FE_CHECK(result == SPV_REFLECT_RESULT_SUCCESS)

namespace fe::rhi::vulkan
{

//============================================================================================================================================================================================
#pragma region [ UTILITY ]
//============================================================================================================================================================================================

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

VkColorSpaceKHR get_color_space(ColorSpace colorSpace)
{
    switch (colorSpace)
    {
    default:
    case (ColorSpace::SRGB):
        return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    case (ColorSpace::HDR10_ST2084):
        return VK_COLOR_SPACE_HDR10_ST2084_EXT;
    case (ColorSpace::HDR_LINEAR):
        return VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT;
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

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ INTERNAL API ]
//============================================================================================================================================================================================

VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* userData)
{
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		FE_LOG(LogVulkanRHI, WARNING, "{}", callbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		FE_LOG(LogVulkanRHI, ERROR, "{}", callbackData->pMessage);
	}

	return VK_FALSE;
}

class Instance
{
public:
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;

    void init(ValidationMode validationMode)
    {
        FE_LOG(LogVulkanRHI, INFO, "Starting Vulkan Instance initialization.");

        VK_CHECK(volkInitialize());

        uint32_t instanceLayerCount;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
        std::vector<VkLayerProperties> availableInstanceLayers(instanceLayerCount);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableInstanceLayers.data()));
        VkApplicationInfo appInfo{};

        uint32_t extensionCount;
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> availableInstanceExtensions(extensionCount);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableInstanceExtensions.data()));

        std::vector<const char*> instanceLayers;
        std::vector<const char*> instanceExtensions;

        bool canUseDebugUtils = false;

        for (auto& availableExtension : availableInstanceExtensions)
        {
            if (strcmp(availableExtension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
            {
                canUseDebugUtils = true;
                instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            else if (strcmp(availableExtension.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
            {
                instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            }
            else if (strcmp(availableExtension.extensionName, VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME) == 0)
            {
                instanceExtensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
            }
            else if (strcmp(availableExtension.extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
            {
                instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
            }
    #if defined(VK_USE_PLATFORM_WIN32_KHR)
            else if (strcmp(availableExtension.extensionName, VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
            {
                instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
            }
    #endif
            else if (strcmp(availableExtension.extensionName, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME) == 0)
            {
                instanceExtensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
            }
        }

        if (validationMode != rhi::ValidationMode::DISABLED)
        {
            // Took from https://github.com/turanszkij/WickedEngine/blob/1c49e4984a7f63be44d06d7b7c0949386d531bd8/WickedEngine/wiGraphicsDevice_Vulkan.cpp
            static const std::vector<const char*> validationLayersPriority[] =
            {
                { "VK_LAYER_KHRONOS_validation" },
                { "VK_LAYER_LUNARG_standard_validation" },
                {
                    "VK_LAYER_GOOGLE_threading",
                    "VK_LAYER_LUNARG_parameter_validation",
                    "VK_LAYER_LUNARG_object_tracker",
                    "VK_LAYER_LUNARG_core_validation",
                    "VK_LAYER_GOOGLE_unique_objects",
                },
                { "VK_LAYER_LUNARG_core_validation" }	
            };

            for (auto& validationLayers : validationLayersPriority)
            {
                if (validate_layers(validationLayers, availableInstanceLayers))
                {
                    for (auto& layer : validationLayers)
                    {
                        instanceLayers.push_back(layer);
                    }
                    break;
                }
            }
        }
        
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Fablex Engine Application";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Fablex Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
        instanceCreateInfo.enabledLayerCount = instanceLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();

        VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};
        debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        if (validationMode != rhi::ValidationMode::DISABLED && canUseDebugUtils)
        {
            debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
            debugUtilsCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

            if (validationMode == rhi::ValidationMode::VERBOSE)
            {
                debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
                debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
            }

            debugUtilsCreateInfo.pfnUserCallback = debug_utils_messenger_callback;
            instanceCreateInfo.pNext = &debugUtilsCreateInfo;
        }
        
        VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));

        volkLoadInstanceOnly(instance);

        if (validationMode != rhi::ValidationMode::DISABLED && canUseDebugUtils)
        {
            VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsCreateInfo, nullptr, &debugUtilsMessenger));
        }

        FE_LOG(LogVulkanRHI, INFO, "Vulkan Instance initialization completed.");
    }

    void cleanup()
    {
        FE_LOG(LogVulkanRHI, INFO, "Starting Vulkan Instance cleanup.");

        vkDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessenger, nullptr);
        vkDestroyInstance(instance, nullptr);

        FE_LOG(LogVulkanRHI, INFO, "Vulkan Instance cleanup completed.");
    }

private:
    bool validate_layers(const std::vector<const char*>& requiredLayers, const std::vector<VkLayerProperties>& availableLayers)
    {
        for (auto& requiredLayer : requiredLayers)
	    {
            bool found = false;
            for (auto& availableLayer : availableLayers)
            {
                if (strcmp(requiredLayer, availableLayer.layerName) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                return false;
            }
        }
        return true;
    }
} static g_instance;

class Device
{
public:
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    VkPhysicalDeviceProperties2 properties2{};
    VkPhysicalDeviceVulkan11Properties properties1_1{};
    VkPhysicalDeviceVulkan12Properties properties1_2{};
    VkPhysicalDeviceVulkan13Properties properties1_3{};
    VkPhysicalDeviceMemoryProperties2 memoryProperties2{};
    VkPhysicalDeviceMultiviewProperties multiviewProperties{};
    VkPhysicalDeviceSamplerFilterMinmaxProperties samplerMinMaxProperties{};
    VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties{};
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties{};
    VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties{};
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProperties{};

    VkPhysicalDeviceFeatures2 features2{};
    VkPhysicalDeviceVulkan11Features features1_1{};
    VkPhysicalDeviceVulkan12Features features1_2{};
    VkPhysicalDeviceVulkan13Features features1_3{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
    VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures{};
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};

    std::vector<VkQueueFamilyProperties2> queueFamiliesProperties;

    GPUProperties gpuProperties;

    struct Queue
    {
        uint32 family = VK_QUEUE_FAMILY_IGNORED;
        VkQueue handle = VK_NULL_HANDLE;
        bool isSparseBindingSupported{false};

        void init(VkDevice device, const std::string& debugName)
        {
            VkDeviceQueueInfo2 queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
            queueInfo.queueIndex = 0;
            queueInfo.queueFamilyIndex = family;
            vkGetDeviceQueue2(device, &queueInfo, &handle);

            VkDebugUtilsObjectNameInfoEXT info{};
            info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            info.pObjectName = debugName.c_str();
            info.objectHandle = (uint64)handle;
            info.objectType = VK_OBJECT_TYPE_QUEUE;
            vkSetDebugUtilsObjectNameEXT(device, &info);
        }
    };

    Queue graphicsQueue;
    Queue computeQueue;
    Queue transferQueue;
    std::vector<uint32> queueFamilies;

    void init(GPUPreference gpuPreference)
    {
        FE_LOG(LogVulkanRHI, INFO, "Starting Vulkan Device initialization.");

        uint32_t deviceCount;
        VK_CHECK(vkEnumeratePhysicalDevices(g_instance.instance, &deviceCount, nullptr));
        if (deviceCount == 0)
        {
            FE_LOG(LogVulkanRHI, FATAL, "Physical device count is 0");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        VK_CHECK(vkEnumeratePhysicalDevices(g_instance.instance, &deviceCount, devices.data()));

        std::vector<const char*> enabledExtensions;
        
        for (VkPhysicalDevice device : devices)
        {
            if (!is_physical_device_valid(device, enabledExtensions))
                continue;

            bool priority = properties2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            
            if (gpuPreference == GPUPreference::INTEGRATED)
            {
                priority = properties2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
            }
            if (priority || physicalDevice == VK_NULL_HANDLE)
            {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
        {
            FE_LOG(LogVulkanRHI, FATAL, "Failed to pick physical device");
        }

        FE_CHECK(properties2.properties.limits.timestampComputeAndGraphics == VK_TRUE);
        FE_CHECK(features2.features.imageCubeArray == VK_TRUE);
        FE_CHECK(features2.features.geometryShader == VK_TRUE);
        FE_CHECK(features2.features.textureCompressionBC == VK_TRUE);
        FE_CHECK(features2.features.samplerAnisotropy == VK_TRUE);
        FE_CHECK(features2.features.multiViewport == VK_TRUE);
        FE_CHECK(features2.features.pipelineStatisticsQuery == VK_TRUE);
        FE_CHECK(features2.features.fragmentStoresAndAtomics == VK_TRUE);
        FE_CHECK(features1_2.runtimeDescriptorArray == VK_TRUE);
        FE_CHECK(features1_2.descriptorIndexing == VK_TRUE);
        FE_CHECK(features1_3.synchronization2 == VK_TRUE);
        FE_CHECK(features1_3.dynamicRendering == VK_TRUE);
        FE_CHECK(features1_3.maintenance4 == VK_TRUE);

        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, nullptr);
        queueFamiliesProperties.resize(queueFamilyCount);
        for (uint32_t i = 0; i != queueFamilyCount; ++i)
        {
            queueFamiliesProperties[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
            queueFamiliesProperties[i].pNext = nullptr;	// will use for video queue in the future
        }
        vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, queueFamiliesProperties.data());

        for (uint32_t i = 0; i != queueFamilyCount; ++i)
        {
            const auto& queueFamilyProperties = queueFamiliesProperties[i].queueFamilyProperties;
            if (graphicsQueue.family == VK_QUEUE_FAMILY_IGNORED && queueFamilyProperties.queueCount > 0 && queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                graphicsQueue.family = i;
                if (queueFamilyProperties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                {
                    graphicsQueue.isSparseBindingSupported = true;
                }
            }
            if (computeQueue.family == VK_QUEUE_FAMILY_IGNORED && queueFamilyProperties.queueCount > 0 && queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                computeQueue.family = i;
            }
            if (transferQueue.family == VK_QUEUE_FAMILY_IGNORED && queueFamilyProperties.queueCount > 0 && queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                transferQueue.family = i;
            }
        }

        for (uint32_t i = 0; i != queueFamilyCount; ++i)
        {
            const auto& queueFamilyProperties = queueFamiliesProperties[i].queueFamilyProperties;

            if (queueFamilyProperties.queueCount > 0 &&
                queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT &&
                !(queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                !(queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT))
            {
                transferQueue.family = i;
                if (queueFamilyProperties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                {
                    transferQueue.isSparseBindingSupported = true;
                }
            }

            if (queueFamilyProperties.queueCount > 0 &&
                queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT &&
                !(queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                computeQueue.family = i;
                if (queueFamilyProperties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                {
                    computeQueue.isSparseBindingSupported = true;
                }
            }
        }

        queueFamilies = {graphicsQueue.family, computeQueue.family, transferQueue.family};
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        const float queuePriority = 1.0f;
        for (const auto queueFamily : queueFamilies)
        {
            VkDeviceQueueCreateInfo& createInfo = queueCreateInfos.emplace_back();
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            createInfo.queueFamilyIndex = queueFamily;
            createInfo.queueCount = 1;
            createInfo.pQueuePriorities = &queuePriority;
        }

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.pEnabledFeatures = nullptr;
        deviceCreateInfo.pNext = &features2;
        deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

        VK_CHECK(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));

        volkLoadDevice(device);

        graphicsQueue.init(device, "GraphicsQueue");
        computeQueue.init(device, "ComputeQueue");
        transferQueue.init(device, "TransferQueue");

        fill_gpu_properties();

        FE_LOG(LogVulkanRHI, INFO, "Vulkan Device initialization completed.");
    }

    const Queue& get_queue(QueueType type)
    {
        switch (type)
        {
        default:
        case QueueType::GRAPHICS:
            return graphicsQueue;
        case QueueType::COMPUTE:
            return computeQueue;
        case QueueType::TRANSFER:
            return transferQueue;
        }
    }

    void cleanup()
    {
        FE_LOG(LogVulkanRHI, INFO, "Starting Vulkan Device cleanup.");

        vkDestroyDevice(device, nullptr);

        FE_LOG(LogVulkanRHI, INFO, "Vulkan Device cleanup completed.");
    }

private:
    bool is_physical_device_valid(VkPhysicalDevice physDevice, std::vector<const char*>& outEnabledExtensions)
    {
        static const std::vector<const char*> requiredExtensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        uint32_t extensionCount;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        VK_CHECK(vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &extensionCount, availableExtensions.data()));

        for (auto& requiredExtension : requiredExtensions)
        {
            if (!is_extension_supported(requiredExtension, availableExtensions))
            {
                return false;
            }
        }

        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        features1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        features1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        features2.pNext = &features1_1;
        features1_1.pNext = &features1_2;
        features1_2.pNext = &features1_3;
        void** featuresChain = &features1_3.pNext;

        properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        properties1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
        properties1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
        properties1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
        properties2.pNext = &properties1_1;
        properties1_1.pNext = &properties1_2;
        properties1_2.pNext = &properties1_3;
        void** propertiesChain = &properties1_3.pNext;
        
        multiviewProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES;
        add_to_chain(&propertiesChain, &multiviewProperties);

        samplerMinMaxProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES;
        add_to_chain(&propertiesChain, &samplerMinMaxProperties);

        outEnabledExtensions = requiredExtensions;

        if (is_extension_supported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, availableExtensions))
        {
            outEnabledExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
            assert(is_extension_supported(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, availableExtensions));
            outEnabledExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

            accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
            add_to_chain(&featuresChain, &accelerationStructureFeatures);

            accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
            add_to_chain(&propertiesChain, &accelerationStructureProperties);

            if (is_extension_supported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, availableExtensions))
            {
                outEnabledExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
                outEnabledExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);

                rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
                add_to_chain(&featuresChain, &rayTracingPipelineFeatures);

                rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
                add_to_chain(&propertiesChain, &rayTracingPipelineProperties);
            }

            if (is_extension_supported(VK_KHR_RAY_QUERY_EXTENSION_NAME, availableExtensions))
            {
                outEnabledExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);

                rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
                add_to_chain(&featuresChain, &rayQueryFeatures);
            }
        }
        
        if (is_extension_supported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, availableExtensions))
        {
            outEnabledExtensions.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

            fragmentShadingRateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
            add_to_chain(&featuresChain, &fragmentShadingRateFeatures);

            fragmentShadingRateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
            add_to_chain(&propertiesChain, &fragmentShadingRateProperties);
        }

        if (is_extension_supported(VK_EXT_MESH_SHADER_EXTENSION_NAME, availableExtensions))
        {
            outEnabledExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);

            meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
            add_to_chain(&featuresChain, &meshShaderFeatures);

            meshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
            add_to_chain(&propertiesChain, &meshShaderProperties);
        }

        *propertiesChain = nullptr;
        *featuresChain = nullptr;
        vkGetPhysicalDeviceProperties2(physDevice, &properties2);
        vkGetPhysicalDeviceFeatures2(physDevice, &features2);
        
        memoryProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        vkGetPhysicalDeviceMemoryProperties2(physDevice, &memoryProperties2);

        return true;
    }

    bool is_extension_supported(const char* requiredExtension, const std::vector<VkExtensionProperties>& availableExtensions)
    {
        for (auto& availableExtension : availableExtensions)
        {
            if (strcmp(requiredExtension, availableExtension.extensionName) == 0)
            {
                FE_LOG(LogVulkanRHI, INFO, "{} is supported", requiredExtension);
                return true;
            }
        }
        return false;
    }

    template<typename T>
    void add_to_chain(void*** chain, T* obj)
    {
        **chain = obj;
        *chain = &obj->pNext;
    }

    void fill_gpu_properties()
    {
        gpuProperties.timestampFrequency = uint64(1.0 / (double)properties2.properties.limits.timestampPeriod * 1000 * 1000 * 1000);

        gpuProperties.vendorID = properties2.properties.vendorID;
        gpuProperties.deviceID = properties2.properties.deviceID;
        gpuProperties.gpuName = properties2.properties.deviceName;
        gpuProperties.driverDescription = properties1_2.driverName;
        if (properties1_2.driverInfo[0] != '\0')
        {
            gpuProperties.driverDescription += std::string(": ") + properties1_2.driverInfo;
        }
    
        switch (properties2.properties.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                gpuProperties.gpuType = rhi::GPUType::INTEGRATED;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                gpuProperties.gpuType = rhi::GPUType::DISCRETE;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                gpuProperties.gpuType = rhi::GPUType::VIRTUAL;
                break;
            default:
                gpuProperties.gpuType = rhi::GPUType::OTHER;
                break;
        }
    
        if (features2.features.tessellationShader == VK_TRUE)
        {
            FE_LOG(LogVulkanRHI, INFO, "GPU has tessellation shader capability");
            gpuProperties.capabilities |= rhi::GPUCapability::TESSELLATION;
        }
        if (
            rayTracingPipelineFeatures.rayTracingPipeline == VK_TRUE &&
            rayQueryFeatures.rayQuery == VK_TRUE &&
            accelerationStructureFeatures.accelerationStructure == VK_TRUE &&
            features1_2.bufferDeviceAddress == VK_TRUE)
        {
            FE_LOG(LogVulkanRHI, INFO, "GPU has ray tracing capability");
            gpuProperties.capabilities |= rhi::GPUCapability::RAY_TRACING;
            gpuProperties.shaderIdentifierSize = rayTracingPipelineProperties.shaderGroupHandleSize;
            gpuProperties.accelerationStructureInstanceSize = sizeof(VkAccelerationStructureInstanceKHR);
        }
        if (meshShaderFeatures.meshShader == VK_TRUE && meshShaderFeatures.taskShader == VK_TRUE)
        {
            FE_LOG(LogVulkanRHI, INFO, "GPU has task shader and mesh shader capabilities");
            gpuProperties.capabilities |= rhi::GPUCapability::MESH_SHADER;
        }
        if (fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
        {
            FE_LOG(LogVulkanRHI, INFO, "GPU has variable rate shading capability");
            gpuProperties.capabilities |= rhi::GPUCapability::VARIABLE_RATE_SHADING;
        }
        if (fragmentShadingRateFeatures.attachmentFragmentShadingRate == VK_TRUE)
        {
            FE_LOG(LogVulkanRHI, INFO, "GPU has varialbe rate shading tier 2 capability");
            gpuProperties.capabilities |= rhi::GPUCapability::VARIABLE_RATE_SHADING_TIER2;
        }
        if (fragmentShadingRateProperties.fragmentShadingRateWithFragmentShaderInterlock == VK_TRUE)
        {
            FE_LOG(LogVulkanRHI, INFO, "GPU has fragment shader intelock capability");
            gpuProperties.capabilities |= rhi::GPUCapability::FRAGMENT_SHADER_INTERLOCK;
        }
        if (features2.features.sparseBinding == VK_TRUE && features2.features.sparseResidencyAliased == VK_TRUE)
        {
            if (properties2.properties.sparseProperties.residencyNonResidentStrict == VK_TRUE)
            {
                FE_LOG(LogVulkanRHI, INFO, "GPU has sparse null mapping capability");
                gpuProperties.capabilities |= rhi::GPUCapability::SPARSE_NULL_MAPPING;
            }
            if (features2.features.sparseResidencyBuffer == VK_TRUE)
            {
                FE_LOG(LogVulkanRHI, INFO, "GPU has sparse buffer capability");
                gpuProperties.capabilities |= rhi::GPUCapability::SPARSE_BUFFER;
            }
            if (features2.features.sparseResidencyImage2D == VK_TRUE)
            {
                FE_LOG(LogVulkanRHI, INFO, "GPU has sparse texture2D capability");
                gpuProperties.capabilities |= rhi::GPUCapability::SPARSE_TEXTURE2D;
            }
            if (features2.features.sparseResidencyImage3D == VK_TRUE)
            {
                FE_LOG(LogVulkanRHI, INFO, "GPU has sparse texture3D capability");
                gpuProperties.capabilities |= rhi::GPUCapability::SPARSE_TEXTURE3D;
            }
            FE_LOG(LogVulkanRHI, INFO, "GPU has sparse tile pool capability");
            gpuProperties.capabilities |= rhi::GPUCapability::SPARSE_TILE_POOL;
        }
        const VkPhysicalDeviceMemoryProperties& memoryProperties = memoryProperties2.memoryProperties;
        for (uint32_t i = 0; i != memoryProperties.memoryHeapCount; ++i)
        {
            if (memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            {
                for (uint32_t j = 0; j != memoryProperties.memoryTypeCount; ++j)
                {
                    if (memoryProperties.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT &&
                        memoryProperties.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                    {
                        FE_LOG(LogVulkanRHI, INFO, "GPU has cache coherent UMA capability");
                        gpuProperties.capabilities |= rhi::GPUCapability::CACHE_COHERENT_UMA;
                        break;
                    }
                }
            }
            if (has_flag(gpuProperties.capabilities, rhi::GPUCapability::CACHE_COHERENT_UMA))
            {
                break;
            }
        }
    }
} static g_device;

class Allocator
{
public:
    ThreadSafePoolAllocator<SwapChain, 4> swapChainAllocator;
    ThreadSafePoolAllocator<Pipeline, 1024> pipelineAllocator;
    ThreadSafePoolAllocator<Shader, 1024> shaderAllocator;
    ThreadSafePoolAllocator<Buffer, 512> bufferAllocator;
    ThreadSafePoolAllocator<Texture, 256> textureAllocator;
    ThreadSafePoolAllocator<TextureView, 512> textureViewAllocator;
    ThreadSafePoolAllocator<BufferView, 512> bufferViewAllocator;
    ThreadSafePoolAllocator<Sampler, 256> samplerAllocator;
    ThreadSafePoolAllocator<CommandPool, 18> cmdPoolAllocator;
    ThreadSafePoolAllocator<CommandBuffer, 64> cmdBufferAllocator;
    ThreadSafePoolAllocator<Semaphore, 24> semaphoreAllocator;
    ThreadSafePoolAllocator<Fence, 8> fenceAllocator;
    ThreadSafePoolAllocator<AccelerationStructure, 512> accelerationStructureAllocator;

    VmaAllocator gpuAllocator = VK_NULL_HANDLE;

    void init()
    {
        FE_LOG(LogVulkanRHI, INFO, "Starting Vulkan Allocator initialization.");

        VmaVulkanFunctions vmaVulkanFunctions{};
        vmaVulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vmaVulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        vmaVulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        vmaVulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        vmaVulkanFunctions.vkAllocateMemory = vkAllocateMemory;
        vmaVulkanFunctions.vkFreeMemory = vkFreeMemory;
        vmaVulkanFunctions.vkMapMemory = vkMapMemory;
        vmaVulkanFunctions.vkUnmapMemory = vkUnmapMemory;
        vmaVulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
        vmaVulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
        vmaVulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
        vmaVulkanFunctions.vkBindImageMemory = vkBindImageMemory;
        vmaVulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        vmaVulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        vmaVulkanFunctions.vkCreateBuffer = vkCreateBuffer;
        vmaVulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
        vmaVulkanFunctions.vkCreateImage = vkCreateImage;
        vmaVulkanFunctions.vkDestroyImage = vkDestroyImage;
        vmaVulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
        vmaVulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
        vmaVulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;

        VmaAllocatorCreateInfo allocatorCreateInfo{};
        allocatorCreateInfo.instance = g_instance.instance;
        allocatorCreateInfo.physicalDevice = g_device.physicalDevice;
        allocatorCreateInfo.device = g_device.device;
        
        if (g_device.features1_2.bufferDeviceAddress)
        {
            allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
            vmaVulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2;
            vmaVulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2;
        }

        allocatorCreateInfo.pVulkanFunctions = &vmaVulkanFunctions;

        VK_CHECK(vmaCreateAllocator(&allocatorCreateInfo, &gpuAllocator));

        FE_LOG(LogVulkanRHI, INFO, "Vulkan Allocator initialization completed.");
    }
    
    void cleanup()
    {
        FE_LOG(LogVulkanRHI, INFO, "Starting Vulkan Allocator cleanup.");

        vmaDestroyAllocator(gpuAllocator);

        FE_LOG(LogVulkanRHI, INFO, "Vulkan Allocator cleanup completed.");
    }
} static g_allocator;

class DescriptorHeap
{
public:
    static constexpr uint32 s_undefinedDescriptor = ~0u;

    void init()
    {
        const VkPhysicalDeviceVulkan12Properties& properties = g_device.properties1_2;

        m_imageBindlessPool.init(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, std::min(s_maxBindlessDescriptors, properties.maxDescriptorSetUpdateAfterBindSampledImages / 4));
        m_storageImageBindlessPool.init(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, std::min(s_maxBindlessDescriptors, properties.maxDescriptorSetUpdateAfterBindStorageImages / 4));
        m_storageBufferBindlessPool.init(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, std::min(s_maxBindlessDescriptors, properties.maxDescriptorSetUpdateAfterBindStorageBuffers / 4));
        m_uniformTexelBufferBindlessPool.init(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, std::min(s_maxBindlessDescriptors, properties.maxDescriptorSetUpdateAfterBindSampledImages / 4));
        m_storageTexelBufferBindlessPool.init(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, std::min(s_maxBindlessDescriptors, properties.maxDescriptorSetUpdateAfterBindStorageBuffers / 4));
        m_samplerBindlessPool.init(VK_DESCRIPTOR_TYPE_SAMPLER, 256);
        m_accelerationStructureBindlessPool.init(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 32);

        m_zeroDescriptorPool.init();
    }

    void cleanup()
    {
        m_imageBindlessPool.cleanup();
        m_storageBufferBindlessPool.cleanup();
        m_storageImageBindlessPool.cleanup();
        m_uniformTexelBufferBindlessPool.cleanup();
        m_storageTexelBufferBindlessPool.cleanup();
        m_samplerBindlessPool.cleanup();
        m_accelerationStructureBindlessPool.cleanup();
        m_zeroDescriptorPool.cleanup();
    }

    void allocate_descriptor(Buffer* buffer)
    {
        FE_CHECK(buffer);

        if (!has_flag(buffer->bufferUsage, ResourceUsage::STORAGE_BUFFER))
            return;

        buffer->descriptorIndex = m_storageBufferBindlessPool.allocate();

        VkDescriptorBufferInfo bufferDescriptorInfo{buffer->vk().buffer, 0, VK_WHOLE_SIZE};

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.dstSet = m_storageBufferBindlessPool.set;
        writeDescriptorSet.dstArrayElement = buffer->descriptorIndex;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pBufferInfo = &bufferDescriptorInfo;
        vkUpdateDescriptorSets(g_device.device, 1, &writeDescriptorSet, 0, nullptr);
    }

    void allocate_descriptor(BufferView* bufferView)
    {
        FE_CHECK(bufferView);

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

        if (bufferView->format != rhi::Format::UNDEFINED)
        {
            switch (bufferView->type)
            {
            case rhi::ViewType::SRV:
                writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                writeDescriptorSet.dstSet = m_uniformTexelBufferBindlessPool.set;
                bufferView->descriptorIndex = m_uniformTexelBufferBindlessPool.allocate();
                break;
            case rhi::ViewType::UAV:
                writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                writeDescriptorSet.dstSet = m_storageTexelBufferBindlessPool.set;
                bufferView->descriptorIndex = m_storageTexelBufferBindlessPool.allocate();
                break;
            default:
                FE_LOG(LogVulkanRHI, FATAL, "Failed to allocate descriptor for buffer view");
            }

            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.dstArrayElement = bufferView->descriptorIndex;
            writeDescriptorSet.descriptorCount = 1;
            VkBufferView vkBufferViewHandle = bufferView->vk().bufferView;
            writeDescriptorSet.pTexelBufferView = &vkBufferViewHandle;
            vkUpdateDescriptorSets(g_device.device, 1, &writeDescriptorSet, 0, nullptr);
        }
        else
        {
            bufferView->descriptorIndex = m_storageBufferBindlessPool.allocate();

            VkDescriptorBufferInfo bufferDescriptorInfo{
                bufferView->buffer->vk().buffer, 
                bufferView->offset, 
                bufferView->size
            };
    
            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.dstSet = m_storageBufferBindlessPool.set;
            writeDescriptorSet.dstArrayElement = bufferView->descriptorIndex;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.pBufferInfo = &bufferDescriptorInfo;
            vkUpdateDescriptorSets(g_device.device, 1, &writeDescriptorSet, 0, nullptr);
        }
    }

    void allocate_descriptor(TextureView* textureView)
    {
        FE_CHECK(textureView);

        auto setSampledImageDescriptor = [&]()
        {
            if (textureView->descriptorIndex == s_undefinedDescriptor)
            {
                textureView->descriptorIndex = m_imageBindlessPool.allocate();
            }
            
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageView = textureView->vk().imageView;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.dstSet = m_imageBindlessPool.set;
            writeDescriptorSet.dstArrayElement = textureView->descriptorIndex;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.pImageInfo = &imageInfo;
            vkUpdateDescriptorSets(g_device.device, 1, &writeDescriptorSet, 0, nullptr);
        };

        auto setStorageImageDescriptor = [&]()
        {
            if (textureView->descriptorIndex == s_undefinedDescriptor)
            {
                textureView->descriptorIndex = m_storageImageBindlessPool.allocate();
            }
            
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageInfo.imageView = textureView->vk().imageView;

            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.dstSet = m_storageImageBindlessPool.set;
            writeDescriptorSet.dstArrayElement = textureView->descriptorIndex;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.pImageInfo = &imageInfo;
            vkUpdateDescriptorSets(g_device.device, 1, &writeDescriptorSet, 0, nullptr);
        };

        switch (textureView->type)
        {
        case rhi::ViewType::SRV:
        case rhi::ViewType::RTV:
        case rhi::ViewType::DSV:
            setSampledImageDescriptor();
            break;
        case rhi::ViewType::UAV:
            setStorageImageDescriptor();
            break;
        case rhi::ViewType::AUTO:
            if (has_flag(textureView->texture->textureUsage, rhi::ResourceUsage::SAMPLED_TEXTURE))
            {
                setSampledImageDescriptor();
            }
            if (has_flag(textureView->texture->textureUsage, rhi::ResourceUsage::STORAGE_TEXTURE))
            {
                setStorageImageDescriptor();
            }
            break;
        }
    }

    void allocate_descriptor(Sampler* sampler)
    {
        sampler->descriptorIndex = m_samplerBindlessPool.allocate();

        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = sampler->vk().sampler;

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.dstSet = m_samplerBindlessPool.set;
        writeDescriptorSet.dstArrayElement = sampler->descriptorIndex;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pImageInfo = &imageInfo;
        vkUpdateDescriptorSets(g_device.device, 1, &writeDescriptorSet, 0, nullptr);
    }

    void allocate_descriptor(AccelerationStructure* accelerationStructure)
    {
        accelerationStructure->descriptorIndex = m_accelerationStructureBindlessPool.allocate();
        
        VkWriteDescriptorSetAccelerationStructureKHR asInfo{};
        asInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        asInfo.accelerationStructureCount = 1;
        asInfo.pAccelerationStructures = &accelerationStructure->vk().accelerationStructure;

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.dstSet = m_accelerationStructureBindlessPool.set;
        writeDescriptorSet.dstArrayElement = accelerationStructure->descriptorIndex;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pNext = &asInfo;
        vkUpdateDescriptorSets(g_device.device, 1, &writeDescriptorSet, 0, nullptr);
    }

    void free_descriptor(Buffer* buffer)
    {
        m_storageBufferBindlessPool.free(buffer->descriptorIndex);
        buffer->descriptorIndex = s_undefinedDescriptor;
    }

    void free_descriptor(BufferView* bufferView)
    {
        if (bufferView->format != rhi::Format::UNDEFINED)
        {
            switch (bufferView->type)
            {
            case rhi::ViewType::SRV:
                m_uniformTexelBufferBindlessPool.free(bufferView->descriptorIndex);
                break;
            case rhi::ViewType::UAV:
                m_storageTexelBufferBindlessPool.free(bufferView->descriptorIndex);
                break;
            default:
                FE_LOG(LogVulkanRHI, FATAL, "Failed to free descriptor for buffer view");
            }
        }
        else 
        {
            m_storageBufferBindlessPool.free(bufferView->descriptorIndex);
        }

        bufferView->descriptorIndex = s_undefinedDescriptor;
    }

    void free_descriptor(TextureView* textureView)
    {
        switch (textureView->type)
        {
        case rhi::ViewType::SRV:
        case rhi::ViewType::RTV:
        case rhi::ViewType::DSV:
            m_imageBindlessPool.free(textureView->descriptorIndex);
            break;
        case rhi::ViewType::UAV:
            m_storageImageBindlessPool.free(textureView->descriptorIndex);
            break;
        case rhi::ViewType::AUTO:
            if (has_flag(textureView->texture->textureUsage, rhi::ResourceUsage::SAMPLED_TEXTURE))
            {
                m_imageBindlessPool.free(textureView->descriptorIndex);
            }
            if (has_flag(textureView->texture->textureUsage, rhi::ResourceUsage::STORAGE_TEXTURE))
            {
                m_storageImageBindlessPool.free(textureView->descriptorIndex);
            }
            break;
        }

        textureView->descriptorIndex = s_undefinedDescriptor;
    }

    void free_descriptor(Sampler* sampler)
    {
        m_samplerBindlessPool.free(sampler->descriptorIndex);
        sampler->descriptorIndex = s_undefinedDescriptor;
    }

    void free_descriptor(AccelerationStructure* accelerationStructure)
    {
        m_accelerationStructureBindlessPool.free(accelerationStructure->descriptorIndex);
        accelerationStructure->descriptorIndex = s_undefinedDescriptor;
    }

    VkDescriptorSetLayout get_bindless_descriptor_set_layout(VkDescriptorType descriptorType)
    {
        return find_bindless_descriptor_pool(descriptorType).layout;
    }

    VkDescriptorSet get_bindless_descriptor_set(VkDescriptorType descriptorType)
    {
        return find_bindless_descriptor_pool(descriptorType).set;
    }

    struct ZeroDescirptorSetInfo
    {
        VkDescriptorSetLayout layout;
        uint64 hash;
    };

    ZeroDescirptorSetInfo get_zero_descriptor_set_info(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
    {
        uint64 hash = 0;
        uint32 bindingMask = 0;

        for (const VkDescriptorSetLayoutBinding& binding : bindings)
        {
            Utils::hash_combine(hash, binding.binding);
            Utils::hash_combine(hash, binding.descriptorCount);
            Utils::hash_combine(hash, binding.descriptorType);
            Utils::hash_combine(hash, binding.stageFlags);

            bindingMask |= 1 << binding.binding;
        }

        std::scoped_lock<std::mutex> locker(m_zeroDescriptorPool.mutex);

        const auto& it = m_zeroDescriptorPool.descriptorSetsByItsHash.find(hash);
        if (it != m_zeroDescriptorPool.descriptorSetsByItsHash.end())
            return ZeroDescirptorSetInfo{it->second.layout, hash};

        ZeroDescriptorPool::ZeroDescriptorSets newZeroSets;
        newZeroSets.bindingCount = bindings.size();
        newZeroSets.bindingMask = bindingMask;

        VkDescriptorSetLayoutCreateInfo laycreateInfo{};
        laycreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        laycreateInfo.bindingCount = bindings.size();
        laycreateInfo.pBindings = bindings.data();
        VK_CHECK(vkCreateDescriptorSetLayout(g_device.device, &laycreateInfo, nullptr, &newZeroSets.layout));

        m_zeroDescriptorPool.descriptorSetsByItsHash[hash] = newZeroSets;

        return ZeroDescirptorSetInfo{newZeroSets.layout, hash};
    }

    VkDescriptorSet get_zero_descriptor_set(uint64 hash, uint32 frameIndex)
    {
        std::scoped_lock<std::mutex> locker(m_zeroDescriptorPool.mutex);

        const auto& it = m_zeroDescriptorPool.descriptorSetsByItsHash.find(hash);
        if (it == m_zeroDescriptorPool.descriptorSetsByItsHash.end())
        {
            FE_LOG(LogVulkanRHI, ERROR, "Failed to find zero descriptor set using hash {}", hash);
            return VK_NULL_HANDLE;
        }

        ZeroDescriptorPool::ZeroDescriptorSets& zeroDescriptorSets = it->second;
        if (zeroDescriptorSets.descriptorSets.size() < frameIndex + 1)
        {
            zeroDescriptorSets.descriptorSets.emplace_back();
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_zeroDescriptorPool.pool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &zeroDescriptorSets.layout;

            VK_CHECK(vkAllocateDescriptorSets(g_device.device, &allocInfo, &zeroDescriptorSets.descriptorSets[frameIndex]));
        }

        return zeroDescriptorSets.descriptorSets[frameIndex];
    }

    void allocate_uniform_buffer(Buffer* buffer, uint32_t size, uint32_t offset, uint32_t slot, uint32_t frameIndex)
    {
        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = buffer->vk().buffer;
        bufferInfo.offset = offset;
        bufferInfo.range = size;

        std::vector<VkWriteDescriptorSet> writes;
        
        for (auto& [hash, zeroDescriptorSet] : m_zeroDescriptorPool.descriptorSetsByItsHash)
        {
            if (!zeroDescriptorSet.has_binding(slot))
                continue;

            auto& writeDescriptorSet = writes.emplace_back();
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptorSet.dstBinding = slot;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.dstSet = get_zero_descriptor_set(hash, frameIndex);
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.pBufferInfo = &bufferInfo;
        }

        vkUpdateDescriptorSets(g_device.device, writes.size(), writes.data(), 0, nullptr);
    }

private:
    struct BindlessDescriptorPool
    {
        VkDescriptorPool pool = VK_NULL_HANDLE;
        VkDescriptorSet set = VK_NULL_HANDLE;
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        std::vector<uint32> freePlaces;
        std::mutex mutex;

        void init(VkDescriptorType descriptorType, uint32_t descriptorCount)
        {
            VkDescriptorPoolSize poolSize;
            poolSize.type = descriptorType;
            poolSize.descriptorCount = descriptorCount;
            
            VkDescriptorPoolCreateInfo poolCreateInfo{};
            poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
            poolCreateInfo.maxSets = 1;
            poolCreateInfo.poolSizeCount = 1;
            poolCreateInfo.pPoolSizes = &poolSize;
            VK_CHECK(vkCreateDescriptorPool(g_device.device, &poolCreateInfo, nullptr, &pool));

            VkDescriptorSetLayoutBinding binding;
            binding.binding = 0;
            binding.descriptorType = descriptorType;
            binding.descriptorCount = descriptorCount;
            binding.stageFlags = VK_SHADER_STAGE_ALL;
            binding.pImmutableSamplers = nullptr;

            VkDescriptorBindingFlags bindingFlags =
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT |
                VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;
            VkDescriptorSetLayoutBindingFlagsCreateInfo layoutBindingFlags{};
            layoutBindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
            layoutBindingFlags.pBindingFlags = &bindingFlags;
            layoutBindingFlags.bindingCount = 1;

            VkDescriptorSetLayoutCreateInfo laycreateInfo{};
            laycreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            laycreateInfo.bindingCount = 1;
            laycreateInfo.pBindings = &binding;
            laycreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            laycreateInfo.pNext = &layoutBindingFlags;
            VK_CHECK(vkCreateDescriptorSetLayout(g_device.device, &laycreateInfo, nullptr, &layout));

            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &layout;
            allocInfo.descriptorPool = pool;
            VK_CHECK(vkAllocateDescriptorSets(g_device.device, &allocInfo, &set));
            
            for (int32 i = 0; i != (int32)descriptorCount; ++i)
            {
                freePlaces.push_back((int32)descriptorCount - i - 1);
            }
        }

        void cleanup()
        {
            if (pool != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorPool(g_device.device, pool, nullptr);
                pool = VK_NULL_HANDLE;
            }
            if (layout != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorSetLayout(g_device.device, layout, nullptr);
                layout = VK_NULL_HANDLE;
            }
        }

        uint32 allocate()
        {
            std::scoped_lock<std::mutex> locker(mutex);
            if (freePlaces.empty())
                FE_LOG(LogVulkanRHI, FATAL, "There are no free places in the descriptor pool.");
            uint32 index = freePlaces.back();
            freePlaces.pop_back();
            return index;
        }

        void free(uint32 descriptorIndex)
        {
            std::scoped_lock<std::mutex> locker(mutex);
            if (descriptorIndex > 0)
                freePlaces.push_back(descriptorIndex);
        }
    };

    struct ZeroDescriptorPool
    {
        struct ZeroDescriptorSets
        {
            std::vector<VkDescriptorSet> descriptorSets;
            VkDescriptorSetLayout layout = VK_NULL_HANDLE;
            uint32 bindingCount;
            uint32 bindingMask = 0;

            bool has_binding(uint32 binding) const
            {
                return (bindingMask & (1u << binding)) != 0;
            }
        };

        VkDescriptorPool pool = VK_NULL_HANDLE;
        std::unordered_map<uint64, ZeroDescriptorSets> descriptorSetsByItsHash;
        std::mutex mutex;

        void init()
        {
            VkDescriptorPoolSize poolSize;
            poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSize.descriptorCount = s_maxUniformBuffers * s_maxZeroSets;

            VkDescriptorPoolCreateInfo poolCreateInfo{};
            poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolCreateInfo.maxSets = s_maxZeroSets;
            poolCreateInfo.poolSizeCount = 1;
            poolCreateInfo.pPoolSizes = &poolSize;
            VK_CHECK(vkCreateDescriptorPool(g_device.device, &poolCreateInfo, nullptr, &pool));
        }

        void cleanup()
        {
            if (pool != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorPool(g_device.device, pool, nullptr);
            }

            for (auto& it : descriptorSetsByItsHash)
            {
                vkDestroyDescriptorSetLayout(g_device.device, it.second.layout, nullptr);
            }
        }
    };

    const BindlessDescriptorPool& find_bindless_descriptor_pool(VkDescriptorType descriptorType) const
    {
        switch (descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            return m_samplerBindlessPool;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            return m_storageBufferBindlessPool;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            return m_imageBindlessPool;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return m_storageImageBindlessPool;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            return m_uniformTexelBufferBindlessPool;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return m_storageTexelBufferBindlessPool;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return m_accelerationStructureBindlessPool;
        default:
            FE_LOG(LogVulkanRHI, FATAL, "Unsupported descritpor type.");
            return m_samplerBindlessPool;
        }
    }

    static constexpr uint32 s_maxBindlessDescriptors = 100000u;
    static constexpr uint32 s_maxUniformBuffers = 16u;
    static constexpr uint32 s_maxZeroSets = 64;

    BindlessDescriptorPool m_imageBindlessPool;
    BindlessDescriptorPool m_storageImageBindlessPool;
    BindlessDescriptorPool m_samplerBindlessPool;
    BindlessDescriptorPool m_storageBufferBindlessPool;
    BindlessDescriptorPool m_uniformTexelBufferBindlessPool;
    BindlessDescriptorPool m_storageTexelBufferBindlessPool;
    BindlessDescriptorPool m_accelerationStructureBindlessPool;
    ZeroDescriptorPool m_zeroDescriptorPool;
} static g_descriptorHeap;

class ShaderReflectionCache
{
public:
    struct BindlessBindingInfo
    {
        bool isUsed = false;
        VkDescriptorSetLayoutBinding binding;
    };

    struct ShaderReflectionInfo
    {
        std::optional<VkPushConstantRange> pushConstant = std::nullopt;
        std::vector<BindlessBindingInfo> bindlessBindings;
        std::vector<VkDescriptorSetLayoutBinding> zeroSetBindings;
    };

    void reflect(Shader* shader, const ShaderInfo* shaderInfo)
    {
        {
            std::scoped_lock<std::mutex> locker(m_mutex);
            if (m_shaderReflectionInfos.find(shader) != m_shaderReflectionInfos.end())
            {
                return;
            }
        }

        std::unique_ptr<ShaderReflectionInfo> reflectionInfo = std::make_unique<ShaderReflectionInfo>();

        SpvReflectShaderModule reflectModule;
        SpvReflectResult result = spvReflectCreateShaderModule(shaderInfo->size, shaderInfo->data, &reflectModule);
        CHECK_SHADER_REFLECTION(result);

        uint32 bindingCount = 0;
        result = spvReflectEnumerateDescriptorBindings(&reflectModule, &bindingCount, nullptr);
        CHECK_SHADER_REFLECTION(result);
        
        std::vector<SpvReflectDescriptorBinding*> bindings(bindingCount);
        result = spvReflectEnumerateDescriptorBindings(&reflectModule, &bindingCount, bindings.data());
        CHECK_SHADER_REFLECTION(result);

        uint32 pushConstantCount = 0;
        result = spvReflectEnumeratePushConstantBlocks(&reflectModule, &pushConstantCount, nullptr);
        CHECK_SHADER_REFLECTION(result);

        std::vector<SpvReflectBlockVariable*> pushConstants(pushConstantCount);
        result = spvReflectEnumeratePushConstantBlocks(&reflectModule, &pushConstantCount, pushConstants.data());
        CHECK_SHADER_REFLECTION(result);

        for (auto& pushConstant : pushConstants)
        {
            if (!reflectionInfo->pushConstant)
                reflectionInfo->pushConstant = VkPushConstantRange();
            
            reflectionInfo->pushConstant->stageFlags = get_shader_stage(shaderInfo->shaderType);
            reflectionInfo->pushConstant->offset = pushConstant->offset;
            reflectionInfo->pushConstant->size = pushConstant->size;
        }

        for (auto& binding : bindings)
        {
            bool isBindless = binding->set > 0;

            if (isBindless)
            {
                reflectionInfo->bindlessBindings.resize(std::max(reflectionInfo->bindlessBindings.size(), (size_t)binding->set));
                reflectionInfo->bindlessBindings[binding->set - 1].isUsed = true;
            }

            auto& vkBinding = isBindless ? reflectionInfo->bindlessBindings[binding->set - 1].binding : reflectionInfo->zeroSetBindings.emplace_back();
            vkBinding.binding = binding->binding;
            vkBinding.descriptorCount = binding->count;
            vkBinding.descriptorType = (VkDescriptorType)binding->descriptor_type;
            vkBinding.stageFlags = VK_SHADER_STAGE_ALL;
        }

        spvReflectDestroyShaderModule(&reflectModule);

        std::scoped_lock<std::mutex> locker(m_mutex);
        m_shaderReflectionInfos[shader] = std::move(reflectionInfo);
    }

    const ShaderReflectionInfo* get_reflection_info(const Shader* shader) const
    {
        std::scoped_lock<std::mutex> locker(m_mutex);
        const auto& it = m_shaderReflectionInfos.find(shader);
        if (it != m_shaderReflectionInfos.end())
            return it->second.get();

        FE_LOG(LogVulkanRHI, ERROR, "Failed to find shader reflection info->");
        return nullptr;
    }

private:
    mutable std::mutex m_mutex;
    std::unordered_map<const Shader*, std::unique_ptr<ShaderReflectionInfo>> m_shaderReflectionInfos;
} static g_shaderReflectionCache;

class PipelineLayoutCache
{
public:
    struct PipelineLayout
    {
        uint64 layoutHash = 0;
        VkPipelineLayout layout;
        uint64 zeroDescriptorSetHash = 0;
        std::vector<VkDescriptorSet> bindlessDescriptorSets;
        VkPushConstantRange pushConstant;
        uint32 firstBindlessSet;

        void bind_descriptor_sets(VkCommandBuffer cmd, uint32 frameIndex, VkPipelineBindPoint bindPoint) const
        {
            if (zeroDescriptorSetHash)
            {
                VkDescriptorSet descriptorSet = g_descriptorHeap.get_zero_descriptor_set(zeroDescriptorSetHash, frameIndex);
                vkCmdBindDescriptorSets(cmd, bindPoint, layout, 0, 1, &descriptorSet, 0, nullptr);
            }
            if (!bindlessDescriptorSets.empty())
            {
                vkCmdBindDescriptorSets(cmd, bindPoint, layout, firstBindlessSet, bindlessDescriptorSets.size(), bindlessDescriptorSets.data(), 0, nullptr);
            }
        }

        void push_constants(VkCommandBuffer cmd, void* data) const
        {
            vkCmdPushConstants(cmd, layout, pushConstant.stageFlags, pushConstant.offset, pushConstant.size, data);
        }
    };

    void cleanup()
    {
        for (auto& pair : m_pipelineLayoutByHash)
        {
            if (pair.second->layout != VK_NULL_HANDLE)
            {
                vkDestroyPipelineLayout(g_device.device, pair.second->layout, nullptr);
            }
        }
    }

    const PipelineLayout& find_or_add_layout(const std::vector<Shader*>& shaders)
    {
        struct
        {
            std::vector<VkDescriptorSetLayoutBinding> zeroSetBindings;
            std::vector<VkDescriptorSetLayoutBinding> bindlessBindings;
            std::optional<VkPushConstantRange> pushConstant = std::nullopt;
        } layoutInfo;
        
        // Merge reflection info from provided shaders
        for (const Shader* shader : shaders)
        {
            FE_CHECK(shader);
            auto reflectionInfo = g_shaderReflectionCache.get_reflection_info(shader);
            FE_CHECK(reflectionInfo);

            layoutInfo.bindlessBindings.resize(std::max(
                layoutInfo.bindlessBindings.size(),
                reflectionInfo->bindlessBindings.size()
            ));

            for (uint32_t i = 0; i != reflectionInfo->bindlessBindings.size(); ++i)
            {
                auto& reflectInfoBindlessBinding = reflectionInfo->bindlessBindings[i];
                auto& layoutInfoBindlessBinding = layoutInfo.bindlessBindings[i];
                
                if (!reflectInfoBindlessBinding.isUsed)
                    continue;

                if (reflectInfoBindlessBinding.binding.descriptorType != layoutInfoBindlessBinding.descriptorType)
                {
                    layoutInfo.bindlessBindings[i] = reflectInfoBindlessBinding.binding;
                }
                else
                {
                    layoutInfoBindlessBinding.stageFlags |= reflectInfoBindlessBinding.binding.stageFlags;
                }
            }

            layoutInfo.zeroSetBindings.resize(std::max(
                layoutInfo.zeroSetBindings.size(),
                reflectionInfo->zeroSetBindings.size()
            ));

            for (uint32_t i = 0; i != reflectionInfo->zeroSetBindings.size(); ++i)
            {
                auto& reflectContextBinding = reflectionInfo->zeroSetBindings[i];
                auto& interContextBinding = layoutInfo.zeroSetBindings[i];

                if (reflectContextBinding.descriptorType != interContextBinding.descriptorType)
                {
                    layoutInfo.zeroSetBindings[i] = reflectContextBinding;
                }
                else
                {
                    layoutInfo.zeroSetBindings[i].stageFlags |= reflectContextBinding.stageFlags;
                }
            }

            if (reflectionInfo->pushConstant)
            {
                if (!layoutInfo.pushConstant)
                    layoutInfo.pushConstant = VkPushConstantRange();

                layoutInfo.pushConstant->offset = std::min(
                    layoutInfo.pushConstant->offset,
                    reflectionInfo->pushConstant->offset);
                layoutInfo.pushConstant->size = std::max(
                    layoutInfo.pushConstant->size,
                    reflectionInfo->pushConstant->size);
                layoutInfo.pushConstant->stageFlags = VK_SHADER_STAGE_ALL;
            }
        }

        return *find_or_add_layout_internal(
            layoutInfo.zeroSetBindings, 
            layoutInfo.bindlessBindings, 
            layoutInfo.pushConstant
        );
    }

    const PipelineLayout& find_or_add_layout(const Shader* shader)
    {
        FE_CHECK(shader);
        auto reflectionInfo = g_shaderReflectionCache.get_reflection_info(shader);
        FE_CHECK(reflectionInfo);

        std::vector<VkDescriptorSetLayoutBinding> bindlessBindings;
        bindlessBindings.reserve(reflectionInfo->bindlessBindings.size());
        
        for (const auto& bindlessBindingInfo : reflectionInfo->bindlessBindings)
            bindlessBindings.push_back(bindlessBindingInfo.binding);

        return *find_or_add_layout_internal(
            reflectionInfo->zeroSetBindings, 
            bindlessBindings, 
            reflectionInfo->pushConstant
        );
    }

    const PipelineLayout* find_layout(uint64 layoutHash)
    {
        std::scoped_lock<std::mutex> locker(m_mutex);
        auto it = m_pipelineLayoutByHash.find(layoutHash);
        if (it != m_pipelineLayoutByHash.end())
        {
            return it->second.get();
        }
        
        FE_LOG(LogVulkanRHI, ERROR, "Failed to find pipeline layout with hash {}", layoutHash);
        return nullptr;
    }

private:
    std::mutex m_mutex;
    std::unordered_map<uint64, std::unique_ptr<PipelineLayout>> m_pipelineLayoutByHash;

    PipelineLayout* find_or_add_layout_internal(
        const std::vector<VkDescriptorSetLayoutBinding>& zeroSetBindings,
        const std::vector<VkDescriptorSetLayoutBinding>& bindlessBindings,
        const std::optional<VkPushConstantRange>& pushConstant
    )
    {
        uint64 layoutHash = get_layout_hash(zeroSetBindings, bindlessBindings, pushConstant);
        
        {
            std::scoped_lock<std::mutex> locker(m_mutex);
            auto it = m_pipelineLayoutByHash.find(layoutHash);
            if (it != m_pipelineLayoutByHash.end())
            {
                return it->second.get();
            }
        }

        std::unique_ptr<PipelineLayout> layout(new PipelineLayout());
        std::vector<VkDescriptorSetLayout> descriptorLayouts;

        layout->layoutHash = layoutHash;
        layout->pushConstant = pushConstant ? pushConstant.value() : VkPushConstantRange(0, 0, 0);

        DescriptorHeap::ZeroDescirptorSetInfo zeroDescriptorSetInfo = g_descriptorHeap.get_zero_descriptor_set_info(zeroSetBindings);
        descriptorLayouts.push_back(zeroDescriptorSetInfo.layout);
        layout->zeroDescriptorSetHash = zeroDescriptorSetInfo.hash;
        layout->firstBindlessSet = descriptorLayouts.size();

        for (auto& bindlessBinding : bindlessBindings)
        {
            descriptorLayouts.push_back(g_descriptorHeap.get_bindless_descriptor_set_layout(bindlessBinding.descriptorType));
            layout->bindlessDescriptorSets.push_back(g_descriptorHeap.get_bindless_descriptor_set(bindlessBinding.descriptorType));
        }

        VkPipelineLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.pSetLayouts = descriptorLayouts.data();
        createInfo.setLayoutCount = descriptorLayouts.size();
        if (layout->pushConstant.size > 0)
        {
            createInfo.pPushConstantRanges = &layout->pushConstant;
            createInfo.pushConstantRangeCount = 1;
        }
        else
        {
            createInfo.pPushConstantRanges = nullptr;
            createInfo.pushConstantRangeCount = 0;
        }

        VK_CHECK(vkCreatePipelineLayout(g_device.device, &createInfo, nullptr, &layout->layout));

        std::scoped_lock<std::mutex> locker(m_mutex);
        m_pipelineLayoutByHash[layoutHash] = std::move(layout);
        return m_pipelineLayoutByHash[layoutHash].get();
    }

    uint64 get_layout_hash(
        const std::vector<VkDescriptorSetLayoutBinding>& zeroSetBindings,
        const std::vector<VkDescriptorSetLayoutBinding>& bindlessBindings,
        const std::optional<VkPushConstantRange>& pushConstant
    )
    {
        uint64 hash = 0;

        for (auto& binding : zeroSetBindings)
        {
            Utils::hash_combine(hash, binding.binding);
            Utils::hash_combine(hash, binding.descriptorCount);
            Utils::hash_combine(hash, binding.descriptorType);
            Utils::hash_combine(hash, binding.stageFlags);
        }

        for (auto& bindlessBinding : bindlessBindings)
        {
            Utils::hash_combine(hash, bindlessBinding.binding);
            Utils::hash_combine(hash, bindlessBinding.descriptorCount);
            Utils::hash_combine(hash, bindlessBinding.descriptorType);
            Utils::hash_combine(hash, bindlessBinding.stageFlags);
        }

        if (pushConstant)
        {
            Utils::hash_combine(hash, pushConstant->offset);
            Utils::hash_combine(hash, pushConstant->size);
            Utils::hash_combine(hash, pushConstant->stageFlags);
        }

        return hash;
    }
} static g_pipelineLayoutCache;

uint64 g_frameIndex = 0;

VkSurfaceKHR create_surface(const WindowInfo& windowInfo)
{
    VkSurfaceKHR surface;

#ifdef WIN32
    FE_CHECK(windowInfo.win32Window.hWnd);

    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = (HWND)windowInfo.win32Window.hWnd;
    createInfo.hinstance = GetModuleHandle(nullptr);
    if (vkCreateWin32SurfaceKHR(g_instance.instance, &createInfo, nullptr, &surface) != VK_SUCCESS)
        FE_LOG(LogVulkanRHI, FATAL, "Failed to create surface.");
#endif

    return surface;
}

VkSurfaceCapabilitiesKHR get_surface_capabilities(SwapChain* swapChain)
{
    FE_CHECK(swapChain);

    VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    surfaceInfo.surface = swapChain->vk().surface;

    VkSurfaceCapabilities2KHR capabilities2{};
    capabilities2.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilities2KHR(g_device.physicalDevice, &surfaceInfo, &capabilities2));
    return capabilities2.surfaceCapabilities;
}

void get_surface_available_formats(SwapChain* swapChain, std::vector<VkSurfaceFormat2KHR>& outFormats)
{
    FE_CHECK(swapChain);

    VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    surfaceInfo.surface = swapChain->vk().surface;

    uint32_t formatCount;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormats2KHR(g_device.physicalDevice, &surfaceInfo, &formatCount, nullptr));

    outFormats.resize(formatCount);
    for (auto& format : outFormats)
    {
        format.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
    }

    VK_CHECK(vkGetPhysicalDeviceSurfaceFormats2KHR(g_device.physicalDevice, &surfaceInfo, &formatCount, outFormats.data()));
}

void get_surface_available_present_modes(SwapChain* swapChain, std::vector<VkPresentModeKHR>& outPresentModes)
{
    FE_CHECK(swapChain);

    VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    surfaceInfo.surface = swapChain->vk().surface;

    uint32_t presentModeCount;
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModes2EXT(g_device.physicalDevice, &surfaceInfo, &presentModeCount, nullptr));
	outPresentModes.resize(presentModeCount);
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModes2EXT(g_device.physicalDevice, &surfaceInfo, &presentModeCount, outPresentModes.data()));
}

void create_swap_chain_internal(SwapChain* swapChain)
{
    // TODO test swap chain recreation with old swap chain changing color space
    FE_CHECK(swapChain->vk().surface != VK_NULL_HANDLE);

    VkSurfaceCapabilitiesKHR surfaceCapabilities = get_surface_capabilities(swapChain);

    swapChain->bufferCount = std::max(swapChain->bufferCount, surfaceCapabilities.minImageCount);
    if (surfaceCapabilities.maxImageCount > 0 && swapChain->bufferCount > surfaceCapabilities.maxImageCount)
    {
        swapChain->bufferCount = surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = swapChain->vk().surface;
    swapChainCreateInfo.minImageCount = swapChain->bufferCount;
    swapChainCreateInfo.imageFormat = get_format(swapChain->format);
    swapChainCreateInfo.imageColorSpace = get_color_space(swapChain->colorSpace);
    swapChainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;

    std::vector<VkPresentModeKHR> availablePresentModes;
    get_surface_available_present_modes(swapChain, availablePresentModes);

    swapChainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!swapChain->vSync)
    {
        for (auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                swapChainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
            if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                swapChainCreateInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    swapChainCreateInfo.clipped = VK_TRUE;
    swapChainCreateInfo.oldSwapchain = swapChain->vk().swapChain;

    VK_CHECK(vkCreateSwapchainKHR(g_device.device, &swapChainCreateInfo, nullptr, &swapChain->vk().swapChain));

    if (swapChainCreateInfo.oldSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(g_device.device, swapChainCreateInfo.oldSwapchain, nullptr);
    }

    VK_CHECK(vkGetSwapchainImagesKHR(g_device.device, swapChain->vk().swapChain, &swapChain->bufferCount, nullptr));
    swapChain->vk().images.resize(swapChain->bufferCount);
    VK_CHECK(vkGetSwapchainImagesKHR(g_device.device, swapChain->vk().swapChain, &swapChain->bufferCount, swapChain->vk().images.data()));

    for (uint32 i = 0; i != swapChain->vk().images.size(); ++i)
    {
        VkDebugUtilsObjectNameInfoEXT info{};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        std::string name = "SwapChainImage" + std::to_string(i);
        info.pObjectName = name.c_str();
        info.objectType = VK_OBJECT_TYPE_IMAGE;
        info.objectHandle = (uint64)swapChain->vk().images[i];
        VK_CHECK(vkSetDebugUtilsObjectNameEXT(g_device.device, &info));
    }

    for (auto& view : swapChain->vk().imageViews)
        vkDestroyImageView(g_device.device, view, nullptr);

    swapChain->vk().imageViews.resize(swapChain->bufferCount);

    for (size_t i = 0; i != swapChain->bufferCount; ++i)
    {
        VkImageViewCreateInfo viewCreateInfo{};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.image = swapChain->vk().images[i];
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = get_format(swapChain->format);
        viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(g_device.device, &viewCreateInfo, nullptr, &swapChain->vk().imageViews[i]));

        VkDebugUtilsObjectNameInfoEXT info{};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        std::string name = "SwapChainImageView" + std::to_string(i);
        info.pObjectName = name.c_str();
        info.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
        info.objectHandle = (uint64)swapChain->vk().imageViews[i];
        VK_CHECK(vkSetDebugUtilsObjectNameEXT(g_device.device, &info));
    }
}

void execute_image_barrier(CommandBuffer* cmd, VkImageMemoryBarrier2& imageBarrier)
{
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    imageBarrier.subresourceRange.baseArrayLayer = 0;
    imageBarrier.subresourceRange.baseMipLevel = 0;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.memoryBarrierCount = 0;
    dependencyInfo.pMemoryBarriers = nullptr;
    dependencyInfo.pBufferMemoryBarriers = nullptr;
    dependencyInfo.bufferMemoryBarrierCount = 0;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &imageBarrier;
    vkCmdPipelineBarrier2(cmd->vk().cmdBuffer, &dependencyInfo);
}

void add_pre_transfer_image_barrier(CommandBuffer* cmd, Texture* texture)
{
    VkImageMemoryBarrier2 imageBarrier{};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier.srcAccessMask = 0;
    imageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    imageBarrier.image = texture->vk().image;
    imageBarrier.subresourceRange = {
        get_image_aspect(texture->textureUsage),
        0,
        texture->mipLevels,
        0,
        texture->layersCount 
    };

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &imageBarrier;

    vkCmdPipelineBarrier2(cmd->vk().cmdBuffer, &dependencyInfo);   
}

void add_post_transfer_image_barrier(CommandBuffer* cmd, Texture* texture)
{
    VkImageMemoryBarrier2 imageBarrier{};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    imageBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    imageBarrier.image = texture->vk().image;
    imageBarrier.subresourceRange = {
        get_image_aspect(texture->textureUsage),
        0,
        texture->mipLevels,
        0,
        texture->layersCount 
    };

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &imageBarrier;

    vkCmdPipelineBarrier2(cmd->vk().cmdBuffer, &dependencyInfo);
}

VkDeviceAddress get_device_address(VkBuffer buffer)
{
    FE_CHECK(buffer != VK_NULL_HANDLE);

    VkBufferDeviceAddressInfo info{};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    info.buffer = buffer;
    return vkGetBufferDeviceAddress(g_device.device, &info);
}

VkDeviceAddress get_device_address(VkAccelerationStructureKHR accelerationStructure)
{
    FE_CHECK(accelerationStructure != VK_NULL_HANDLE);

    VkAccelerationStructureDeviceAddressInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    info.accelerationStructure = accelerationStructure;
    return vkGetAccelerationStructureDeviceAddressKHR(g_device.device, &info);
}

void fill_blas_geometry(
    const BLAS::Geometry* geometryInfo,
    uint32* outPrimitiveCount,
    VkAccelerationStructureGeometryKHR* outGeometry,
    VkAccelerationStructureBuildRangeInfoKHR* outRange
)
{
    outGeometry->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    outGeometry->flags = 0;

    switch (geometryInfo->type)
    {
    case BLAS::Geometry::TRIANGLES:
    {
        if (has_flag(geometryInfo->flags, BLAS::Geometry::Flags::OPAQUE))
            outGeometry->flags |= VK_GEOMETRY_OPAQUE_BIT_KHR;
        if (has_flag(geometryInfo->flags, BLAS::Geometry::Flags::NO_DUPLICATE_ANYHIT_INVOCATION))
            outGeometry->flags |= VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;

        const BLAS::Geometry::Triangles& trianglesInfo = geometryInfo->triangles;
        VkAccelerationStructureGeometryTrianglesDataKHR& triangles = outGeometry->geometry.triangles;
        
        outGeometry->geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;

        triangles.indexType = VK_INDEX_TYPE_UINT32;
        triangles.indexData.deviceAddress = trianglesInfo.indexBuffer->vk().address +
            trianglesInfo.indexOffset * sizeof(uint32);

        triangles.maxVertex = trianglesInfo.vertexCount;
        triangles.vertexStride = trianglesInfo.vertexStride;
        triangles.vertexFormat = get_format(trianglesInfo.vertexFormat);
        triangles.vertexData.deviceAddress = trianglesInfo.vertexBuffer->vk().address +
            trianglesInfo.vertexOffset;

        if (has_flag(geometryInfo->flags, BLAS::Geometry::Flags::USE_TRANSFORM))
        {
            triangles.transformData.deviceAddress = trianglesInfo.transform3x4Buffer->vk().address;
            
            if (outRange)
                outRange->transformOffset = trianglesInfo.transform3x4BufferOffset;
        }

        if (outPrimitiveCount)
            *outPrimitiveCount = trianglesInfo.indexCount / 3;

        if (outRange)
        {
            outRange->primitiveCount = trianglesInfo.indexCount / 3;
            outRange->primitiveOffset = 0;
        }

        break;
    }
    case BLAS::Geometry::PROCEDURAL_AABBS:
    {
        const BLAS::Geometry::ProceduralAABBs& aabbsInfo = geometryInfo->aabbs;
        VkAccelerationStructureGeometryAabbsDataKHR& aabbs = outGeometry->geometry.aabbs;

        outGeometry->geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
        aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
        aabbs.stride = sizeof(float) * 6.0f;
        aabbs.data.deviceAddress = aabbsInfo.aabbBuffer->vk().address;

        if (outPrimitiveCount)
            *outPrimitiveCount = aabbsInfo.count;

        if (outRange)
        {
            outRange->primitiveCount = aabbsInfo.count;
            outRange->primitiveOffset = aabbsInfo.ofsset;
        }

        break;
    }
    }
}

void fill_tlas_geometry(
    const TLAS* tlasInfo,
    uint32* outPrimitiveCount,
    VkAccelerationStructureGeometryKHR* outGeometry,
    VkAccelerationStructureBuildRangeInfoKHR* outRange
)
{
    outGeometry->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    outGeometry->geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;

    VkAccelerationStructureGeometryInstancesDataKHR& instances = outGeometry->geometry.instances;
    instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    instances.arrayOfPointers = VK_FALSE;
    instances.data.deviceAddress = tlasInfo->instanceBuffer->vk().address;

    if (outPrimitiveCount)
        *outPrimitiveCount = tlasInfo->count;

    if (outRange)
    {
        outRange->primitiveCount = tlasInfo->count;
        outRange->primitiveOffset = tlasInfo->offset;
    }
}

void fill_acceleration_structure_build_geometry_info(
    const AccelerationStructure* dstAccelerationStructure,
    const AccelerationStructure* srcAccelerationStructure,
    VkAccelerationStructureBuildGeometryInfoKHR* outBuildInfo,
    std::vector<VkAccelerationStructureGeometryKHR>* outGeometries,
    std::vector<uint32>* outPrimitivesCount,
    std::vector<VkAccelerationStructureBuildRangeInfoKHR>* outRanges
)
{
    FE_CHECK(dstAccelerationStructure);
    FE_CHECK(outBuildInfo);
    FE_CHECK(outGeometries);

    const AccelerationStructureInfo& asInfo = dstAccelerationStructure->info;

    outBuildInfo->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;

    if (dstAccelerationStructure->vk().accelerationStructure != VK_NULL_HANDLE)
    {
        outBuildInfo->dstAccelerationStructure = dstAccelerationStructure->vk().accelerationStructure;
        outBuildInfo->srcAccelerationStructure = VK_NULL_HANDLE;
        outBuildInfo->mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;

        outBuildInfo->scratchData.deviceAddress = dstAccelerationStructure->vk().scratchAddress;

        if (srcAccelerationStructure && 
            has_flag(dstAccelerationStructure->info.flags, AccelerationStructureInfo::Flags::ALLOW_UPDATE))
        {
            outBuildInfo->mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
            outBuildInfo->srcAccelerationStructure = srcAccelerationStructure->vk().accelerationStructure;
        }
    }

    outBuildInfo->flags = 0;

    if (has_flag(asInfo.flags, AccelerationStructureInfo::Flags::ALLOW_UPDATE))
        outBuildInfo->flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    if (has_flag(asInfo.flags, AccelerationStructureInfo::Flags::ALLOW_COMPACTION))
        outBuildInfo->flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
    if (has_flag(asInfo.flags, AccelerationStructureInfo::Flags::PREFER_FAST_TRACE))
        outBuildInfo->flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    if (has_flag(asInfo.flags, AccelerationStructureInfo::Flags::PREFER_FAST_BUILD))
        outBuildInfo->flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
    if (has_flag(asInfo.flags, AccelerationStructureInfo::Flags::LOW_MEMORY))
        outBuildInfo->flags |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR;

    switch (asInfo.type)
    {
    case AccelerationStructureInfo::BOTTOM_LEVEL:
    {
        outBuildInfo->type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        outGeometries->reserve(asInfo.blas.geometries.size());

        if (outPrimitivesCount)
            outPrimitivesCount->reserve(asInfo.blas.geometries.size());
        if (outRanges)
            outRanges->reserve(asInfo.blas.geometries.size());

        for (const BLAS::Geometry& geometryInfo : asInfo.blas.geometries)
        {
            fill_blas_geometry(
                &geometryInfo,
                outPrimitivesCount ? &outPrimitivesCount->emplace_back() : nullptr,
                &outGeometries->emplace_back(),
                outRanges ? &outRanges->emplace_back() : nullptr
            );
        }

        break;
    }
    case AccelerationStructureInfo::TOP_LEVEL:
    {
        fill_tlas_geometry(
            &asInfo.tlas, 
            outPrimitivesCount ? &outPrimitivesCount->emplace_back() : nullptr,
            &outGeometries->emplace_back(), 
            outRanges ? &outRanges->emplace_back() : nullptr
        );
        break;
    }
    }

    outBuildInfo->geometryCount = outGeometries->size();
    outBuildInfo->pGeometries = outGeometries->data();
}

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ API ]
//============================================================================================================================================================================================

void init(const RHIInitInfo* initInfo)
{
    FE_LOG(LogVulkanRHI, INFO, "Starting Vulkan RHI initialization.");

    g_instance.init(initInfo->validationMode);
    g_device.init(initInfo->gpuPreference);
    g_descriptorHeap.init();
    g_allocator.init();

    FE_LOG(LogVulkanRHI, INFO, "Vulkan RHI initialization completed.");
}

void cleanup()
{
    FE_LOG(LogVulkanRHI, INFO, "Starting Vulkan RHI cleanup.");

    g_pipelineLayoutCache.cleanup();
    g_descriptorHeap.cleanup();
    g_allocator.cleanup();
    g_device.cleanup();
    g_instance.cleanup();

    FE_LOG(LogVulkanRHI, INFO, "Vulkan RHI cleanup completed.");
}

void create_swap_chain(SwapChain** swapChain, const SwapChainInfo* info)
{
    FE_CHECK(swapChain);
    FE_CHECK(info);
    FE_CHECK(info->window);
    const WindowInfo& windowInfo = info->window->get_info();

    SwapChain* swapChainPtr = g_allocator.swapChainAllocator.allocate();
    FE_CHECK(swapChainPtr);
    swapChainPtr->init_vk();

    swapChainPtr->window = info->window;
    swapChainPtr->vk().surface = create_surface(windowInfo);

    std::vector<VkSurfaceFormat2KHR> availableFormats;
    get_surface_available_formats(swapChainPtr, availableFormats);

    VkFormat desiredFormat = get_format(info->format);
    bool isDesiredFormatAvailable = false;

    for (const VkSurfaceFormat2KHR& availableFormat : availableFormats)
    {
        if (!info->useHDR && availableFormat.surfaceFormat.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            continue;

        if (desiredFormat == availableFormat.surfaceFormat.format)
        {
            swapChainPtr->format = info->format;

            switch (availableFormat.surfaceFormat.colorSpace)
            {
            case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
                swapChainPtr->colorSpace = rhi::ColorSpace::SRGB;
                break;
            case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
                swapChainPtr->colorSpace = rhi::ColorSpace::HDR_LINEAR;
                break;
            case VK_COLOR_SPACE_HDR10_ST2084_EXT:
                swapChainPtr->colorSpace = rhi::ColorSpace::HDR10_ST2084;
                break;
            default:
                FE_LOG(LogVulkanRHI, WARNING, "Available format has non-supported color space. Nonlinear sRGB will be used.");
                swapChainPtr->colorSpace = rhi::ColorSpace::SRGB;
                break;
            }

            isDesiredFormatAvailable = true;
        }
    }

    if (!isDesiredFormatAvailable)
    {
        swapChainPtr->format = Format::B8G8R8A8_UNORM;
        swapChainPtr->colorSpace = ColorSpace::SRGB;
    }

    swapChainPtr->bufferCount = info->bufferCount;
    swapChainPtr->vSync = info->vSync;

    create_swap_chain_internal(swapChainPtr);

    *swapChain = swapChainPtr;
}

void destroy_swap_chain(SwapChain* swapChain)
{
    if (!swapChain)
        return;

    for (auto& view : swapChain->vk().imageViews)
        vkDestroyImageView(g_device.device, view, nullptr);

    if (swapChain->vk().swapChain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(g_device.device, swapChain->vk().swapChain, nullptr);

    if (swapChain->vk().surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(g_instance.instance, swapChain->vk().surface, nullptr);

    g_allocator.swapChainAllocator.free(swapChain);
}

void create_buffer(Buffer** buffer, const BufferInfo* info)
{
    FE_CHECK(buffer);
    FE_CHECK(info);

    Buffer* bufferPtr = g_allocator.bufferAllocator.allocate();
    FE_CHECK(bufferPtr);
    bufferPtr->init_vk();

    bufferPtr->bufferUsage = info->bufferUsage;
    bufferPtr->memoryUsage = info->memoryUsage;
    bufferPtr->flags = info->flags;
    bufferPtr->size = info->size;
    bufferPtr->format = info->format;
    bufferPtr->descriptorIndex = DescriptorHeap::s_undefinedDescriptor;

    VkBufferCreateInfo bufferCreateInfo{};
    VmaAllocationCreateInfo allocCreateInfo{};
    
    bufferCreateInfo.pNext = nullptr;
    bufferCreateInfo.flags = 0;
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = info->size;
    bufferCreateInfo.usage = get_buffer_usage(info->bufferUsage);

    if (g_device.features1_2.bufferDeviceAddress == VK_TRUE)
        bufferCreateInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    if (has_flag(info->flags, rhi::ResourceFlags::RAY_TRACING))
    {
        bufferCreateInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
        bufferCreateInfo.usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    }
    
    if (!bufferCreateInfo.usage)
        FE_LOG(LogVulkanRHI, FATAL, "Invalid buffer usage.");
    
    allocCreateInfo.usage = get_memory_usage(info->memoryUsage);

    switch (info->memoryUsage)
    {
        case rhi::MemoryUsage::CPU:
        case rhi::MemoryUsage::CPU_TO_GPU:
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            break;
        case rhi::MemoryUsage::GPU_TO_CPU:
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
            break;
        default:
            allocCreateInfo.flags = 0;
            break;
    }
    
    VK_CHECK(vmaCreateBuffer(g_allocator.gpuAllocator, &bufferCreateInfo, &allocCreateInfo, &bufferPtr->vk().buffer, &bufferPtr->vk().allocation, nullptr));

    if (has_flag(bufferPtr->bufferUsage, rhi::ResourceUsage::STORAGE_BUFFER))
        g_descriptorHeap.allocate_descriptor(bufferPtr);

    if (bufferCreateInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
        bufferPtr->vk().address = get_device_address(bufferPtr->vk().buffer);

    bufferPtr->mappedData = bufferPtr->vk().allocation->GetMappedData();

    *buffer = bufferPtr;
}

void update_buffer(Buffer* buffer, uint64 size, const void* data)
{
    FE_CHECK(buffer);
    FE_CHECK(data);
    FE_CHECK(size);

    if (buffer->memoryUsage == rhi::MemoryUsage::GPU)
		FE_LOG(LogVulkanRHI, FATAL, "Can't copy data from CPU to buffer if memory usage is VMA_MEMORY_USAGE_GPU_ONLY");

    memcpy(buffer->mappedData, data, size);
}

void destroy_buffer(Buffer* buffer)
{
    if (!buffer)
        return;

    if (buffer->vk().buffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(g_allocator.gpuAllocator, buffer->vk().buffer, buffer->vk().allocation);

    g_descriptorHeap.free_descriptor(buffer);
    g_allocator.bufferAllocator.free(buffer);
}

void create_texture(Texture** texture, const TextureInfo* info)
{
    FE_CHECK(texture);
    FE_CHECK(info);

    if (info->format == Format::UNDEFINED)
        FE_LOG(LogVulkanRHI, FATAL, "create_texture(): Undefined format.");
    
    if (info->textureUsage == ResourceUsage::UNDEFINED)
        FE_LOG(LogVulkanRHI, FATAL, "create_texture(): Undefined texture usage.");
    
    if (info->samplesCount == rhi::SampleCount::UNDEFINED)
        FE_LOG(LogVulkanRHI, FATAL, "create_texture(): Undefined sample count.");
    
    if (info->dimension == rhi::TextureDimension::UNDEFINED)
        FE_LOG(LogVulkanRHI, FATAL, "create_texture(): Undefined texture dimension.");


    Texture* texturePtr = g_allocator.textureAllocator.allocate();
    FE_CHECK(texturePtr);
    texturePtr->init_vk();

    texturePtr->width = info->width;
    texturePtr->height = info->height;
    texturePtr->depth = info->depth;
    texturePtr->mipLevels = info->mipLevels;
    texturePtr->layersCount = info->layersCount;
    texturePtr->format = info->format;
    texturePtr->textureUsage = info->textureUsage;
    texturePtr->memoryUsage = info->memoryUsage;
    texturePtr->dimension = info->dimension;
    texturePtr->flags = info->flags;
    
    VkImageCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.format = get_format(info->format);
    createInfo.arrayLayers = info->layersCount;
    createInfo.mipLevels = info->mipLevels;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    const uint32_t depth = std::max(1u, info->depth);
    createInfo.extent = VkExtent3D{ info->width, info->height, depth };
    createInfo.samples = get_sample_count(info->samplesCount);
    
    if (has_flag(info->flags, rhi::ResourceFlags::CUBE_TEXTURE))
    {
        createInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    if (has_flag(info->textureUsage, rhi::ResourceUsage::STORAGE_TEXTURE) && is_format_srgb(info->format))
    {
        createInfo.flags |= VK_IMAGE_CREATE_EXTENDED_USAGE_BIT;
    }

    if (g_device.queueFamilies.size() > 1)
    {
        createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = g_device.queueFamilies.size();
        createInfo.pQueueFamilyIndices = g_device.queueFamilies.data();
    }
    else
    {
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkImageUsageFlags imgUsage = get_image_usage(info->textureUsage);
    createInfo.usage = imgUsage;
    createInfo.imageType = get_image_type(info->dimension);

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = get_memory_usage(info->memoryUsage);
    
    switch (info->memoryUsage)
    {
    case rhi::MemoryUsage::CPU:
    case rhi::MemoryUsage::CPU_TO_GPU:
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        break;
    case rhi::MemoryUsage::GPU_TO_CPU:
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        break;
    default:
        allocCreateInfo.flags = 0;
        break;
    }

    if (texturePtr->vk().image == VK_NULL_HANDLE)
    {
        vmaDestroyImage(g_allocator.gpuAllocator, texturePtr->vk().image, texturePtr->vk().allocation);
		texturePtr->vk().image = VK_NULL_HANDLE;
    }
    
    VK_CHECK(vmaCreateImage(g_allocator.gpuAllocator, &createInfo, &allocCreateInfo, &texturePtr->vk().image, &texturePtr->vk().allocation, nullptr));

    texturePtr->mappedData = texturePtr->vk().allocation->GetMappedData();

    *texture = texturePtr;
}

void destroy_texture(Texture* texture)
{
    if (!texture)
        return;

    if (texture->vk().image != VK_NULL_HANDLE)
        vmaDestroyImage(g_allocator.gpuAllocator, texture->vk().image, texture->vk().allocation);

    g_allocator.textureAllocator.free(texture);
}

void create_texture_view(TextureView** textureView, const TextureViewInfo* info, const Texture* texture)
{
    FE_CHECK(textureView);
    FE_CHECK(info);
    FE_CHECK(texture);

    TextureView* textureViewPtr = g_allocator.textureViewAllocator.allocate();
    FE_CHECK(textureViewPtr);
    textureViewPtr->init_vk();

    textureViewPtr->texture = texture;
    textureViewPtr->baseMipLevel = info->baseMipLevel;
    textureViewPtr->baseLayer = info->baseLayer;
    textureViewPtr->mipLevels = info->mipLevels;
    textureViewPtr->layerCount = info->layerCount;
    textureViewPtr->aspect = info->aspect;
    textureViewPtr->type = info->type;
    textureViewPtr->format = info->format;
    textureViewPtr->descriptorIndex = DescriptorHeap::s_undefinedDescriptor;

    VkImageUsageFlags imgUsage = get_image_usage(texture->textureUsage);

    VkImageAspectFlags aspectFlags;
    if (info->aspect == TextureAspect::UNDEFINED)
    {
        if ((imgUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
        else
            aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    else
    {
        aspectFlags = get_image_aspect(info->aspect);
    }
    
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    if (texture->dimension == TextureDimension::TEXTURE1D)
    {
        if (texture->layersCount == 1)
        {
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
        }
        else
        {
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        }
    }
    else if (texture->dimension == TextureDimension::TEXTURE2D)
    {
        if (texture->layersCount > 1)
        {
            if (has_flag(texture->flags, rhi::ResourceFlags::CUBE_TEXTURE))
            {
                if (texture->layersCount > 6)
                {
                    createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                }
                else
                {
                    createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
                }
            }
            else
            {
                createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            }
        }
        else
        {
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        }
    }
    else if (texture->dimension == rhi::TextureDimension::TEXTURE3D)
    {
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    }
    else
    {
        FE_LOG(LogVulkanRHI, ERROR, "create_texture_view(): Undefined texture dimension.");
        return;
    }
    
    createInfo.image = texture->vk().image;
    createInfo.format = info->format == rhi::Format::UNDEFINED ? get_format(texture->format) : get_format(info->format);
    createInfo.subresourceRange.baseMipLevel = info->baseMipLevel;
    createInfo.subresourceRange.levelCount = info->mipLevels;
    createInfo.subresourceRange.baseArrayLayer = info->baseLayer;
    createInfo.subresourceRange.layerCount = info->layerCount;
    createInfo.subresourceRange.aspectMask = aspectFlags;

    if (!is_component_mapping_valid(info->componentMapping))
    {
        FE_LOG(LogVulkanRHI, ERROR, "create_texture_view(): Invalid component mapping.");
        return;
    }

    createInfo.components = get_component_mapping(info->componentMapping);

    VK_CHECK(vkCreateImageView(g_device.device, &createInfo, nullptr, &textureViewPtr->vk().imageView));

    g_descriptorHeap.allocate_descriptor(textureViewPtr);

    *textureView = textureViewPtr;
}

void destroy_texture_view(TextureView* textureView)
{
    if (!textureView)
        return;
    
    if (textureView->vk().imageView != VK_NULL_HANDLE)
        vkDestroyImageView(g_device.device, textureView->vk().imageView, nullptr);

    g_descriptorHeap.free_descriptor(textureView);
    g_allocator.textureViewAllocator.free(textureView);
}

void create_buffer_view(BufferView** bufferView, const BufferViewInfo* info, const Buffer* buffer)
{
    FE_CHECK(bufferView);
    FE_CHECK(info);
    FE_CHECK(buffer);

    if (buffer->size < info->offset + info->size)
        FE_LOG(LogVulkanRHI, FATAL, "create_buffer_view(): Buffer view needs more memory than buffer can provide. Buffer size: {}; View offset: {}; View size: {}", buffer->size, info->offset, info->size);

    if (!has_flag(buffer->bufferUsage, rhi::ResourceUsage::STORAGE_TEXEL_BUFFER) ||
        !has_flag(buffer->bufferUsage, rhi::ResourceUsage::UNIFORM_TEXEL_BUFFER))
    {
        FE_LOG(LogVulkanRHI, FATAL, "create_buffer_view(): Buffer usage is neither STORAGE_TEXEL_BUFFER nor UNIFORM_TEXEL_BUFFER");
    }

    BufferView* bufferViewPtr = g_allocator.bufferViewAllocator.allocate();
    FE_CHECK(bufferViewPtr);
    bufferViewPtr->init_vk();

    bufferViewPtr->buffer = buffer;
    bufferViewPtr->offset = info->offset;
    bufferViewPtr->size = info->size;
    bufferViewPtr->type = info->type;
    bufferViewPtr->descriptorIndex = DescriptorHeap::s_undefinedDescriptor;

    bufferViewPtr->format = buffer->format;
    if (info->newFormat != Format::UNDEFINED)
        bufferViewPtr->format = info->newFormat;

    if (is_format_srgb(bufferViewPtr->format))
        bufferViewPtr->format = get_non_srgb_format(bufferViewPtr->format);

    if (bufferViewPtr->format != rhi::Format::UNDEFINED)
    {
        VkBufferViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        createInfo.buffer = buffer->vk().buffer;
        createInfo.offset = info->offset;
        createInfo.range = info->size;
        createInfo.format = get_format(bufferViewPtr->format);
        VK_CHECK(vkCreateBufferView(g_device.device, &createInfo, nullptr, &bufferViewPtr->vk().bufferView));
    }
    
    g_descriptorHeap.allocate_descriptor(bufferViewPtr);

    *bufferView = bufferViewPtr;
}

void destroy_buffer_view(BufferView* bufferView)
{
    if (!bufferView)
        return;

    if (bufferView->vk().bufferView != VK_NULL_HANDLE)
        vkDestroyBufferView(g_device.device, bufferView->vk().bufferView, nullptr);

    g_descriptorHeap.free_descriptor(bufferView);
    g_allocator.bufferViewAllocator.free(bufferView);
}

void create_sampler(Sampler** sampler, const SamplerInfo* info)
{
    FE_CHECK(sampler);
    FE_CHECK(info);

    Sampler* samplerPtr = g_allocator.samplerAllocator.allocate();
    FE_CHECK(samplerPtr);
    samplerPtr->init_vk();

    samplerPtr->descriptorIndex = DescriptorHeap::s_undefinedDescriptor;

    if (info->addressMode == rhi::AddressMode::UNDEFINED)
        FE_LOG(LogVulkanRHI, FATAL, "create_sampler(): Undefined address mode. Failed to create VkSampler");
    
    if (info->filter == rhi::Filter::UNDEFINED)
        FE_LOG(LogVulkanRHI, FATAL, "create_sampler(): Undefined filter. Failed to create VkSampler");

    VkSamplerCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    get_filter(info->filter, createInfo);
    createInfo.addressModeU = get_address_mode(info->addressMode);
    createInfo.addressModeV = createInfo.addressModeU;
    createInfo.addressModeW = createInfo.addressModeU;
    createInfo.minLod = info->minLod;
    createInfo.maxLod = VK_LOD_CLAMP_NONE;
    createInfo.mipLodBias = 0.0f;
    if (createInfo.anisotropyEnable == VK_TRUE)
        createInfo.maxAnisotropy = info->maxAnisotropy;
    if (info->borderColor != rhi::BorderColor::UNDEFINED)
        createInfo.borderColor = get_border_color(info->borderColor);

    // Using of min max sampler filter
    VkSamplerReductionModeCreateInfo reductionMode{};
    reductionMode.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO;
    switch (info->filter)
    {
        case Filter::MINIMUM_MIN_MAG_MIP_NEAREST:
        case Filter::MINIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
        case Filter::MINIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
        case Filter::MINIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
        case Filter::MINIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
        case Filter::MINIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
        case Filter::MINIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
        case Filter::MINIMUM_MIN_MAG_MIP_LINEAR:
        case Filter::MINIMUM_ANISOTROPIC:
            reductionMode.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;
            createInfo.pNext = &reductionMode;
            break;
        case Filter::MAXIMUM_MIN_MAG_MIP_NEAREST:
        case Filter::MAXIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
        case Filter::MAXIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
        case Filter::MAXIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
        case Filter::MAXIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
        case Filter::MAXIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
        case Filter::MAXIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
        case Filter::MAXIMUM_MIN_MAG_MIP_LINEAR:
        case Filter::MAXIMUM_ANISOTROPIC:
            reductionMode.reductionMode = VK_SAMPLER_REDUCTION_MODE_MAX;
            createInfo.pNext = &reductionMode;
            break;
        default:
            createInfo.pNext = nullptr;
            break;
    }

    VK_CHECK(vkCreateSampler(g_device.device, &createInfo, nullptr, &samplerPtr->vk().sampler));

    g_descriptorHeap.allocate_descriptor(samplerPtr);

    *sampler = samplerPtr;
}

void destroy_sampler(Sampler* sampler)
{
    if (!sampler)
        return;

    if (sampler->vk().sampler != VK_NULL_HANDLE)
        vkDestroySampler(g_device.device, sampler->vk().sampler, nullptr);

    g_descriptorHeap.free_descriptor(sampler);
    g_allocator.samplerAllocator.free(sampler);
}

void create_shader(Shader** shader, const ShaderInfo* info)
{
    FE_CHECK(shader);
    FE_CHECK(info);

    Shader* shaderPtr = g_allocator.shaderAllocator.allocate();
    FE_CHECK(shaderPtr);
    shaderPtr->init_vk();

    shaderPtr->type = info->shaderType;

    VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;
	
	createInfo.codeSize = info->size;
	createInfo.pCode = reinterpret_cast<uint32*>(info->data);

	if (vkCreateShaderModule(g_device.device, &createInfo, nullptr, &shaderPtr->vk().shader) != VK_SUCCESS)
	{
		FE_LOG(LogVulkanRHI, ERROR, "create_shader(): Failed to create shader module.");
		return;
	}

    g_shaderReflectionCache.reflect(shaderPtr, info);

    *shader = shaderPtr;
}

void destroy_shader(Shader* shader)
{
    if (!shader)
        return;

    if (shader->vk().shader != VK_NULL_HANDLE)
        vkDestroyShaderModule(g_device.device, shader->vk().shader, nullptr);

    g_allocator.shaderAllocator.free(shader);
}

void create_graphics_pipelines(const std::vector<GraphicsPipelineInfo>& infos, std::vector<Pipeline*>& outPipelines)
{
    outPipelines.reserve(outPipelines.size() + infos.size());

    for (const GraphicsPipelineInfo& info : infos)
    {
        create_graphics_pipeline(&outPipelines.emplace_back(), &info);
    }
}

void create_graphics_pipeline(Pipeline** pipeline, const GraphicsPipelineInfo* info)
{
    FE_CHECK(pipeline);
    FE_CHECK(info);

    VkPipelineInputAssemblyStateCreateInfo assemblyState{};
    assemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assemblyState.topology = get_primitive_topology(info->assemblyState.topologyType);
    assemblyState.primitiveRestartEnable = VK_FALSE;

    // Only dynamic viewports, no static
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pScissors = nullptr;
    viewportState.scissorCount = 1;
    viewportState.pViewports = nullptr;
    viewportState.viewportCount = 1;

    if (info->rasterizationState.cullMode == rhi::CullMode::UNDEFINED)
        FE_LOG(LogVulkanRHI, FATAL, "create_graphics_pipeline(): Undefined cull mode. Failed to create VkPipeline.");
    
    if (info->rasterizationState.polygonMode == rhi::PolygonMode::UNDEFINED)
        FE_LOG(LogVulkanRHI, FATAL, "VulkanPipeline::create_graphics_pipeline(): Undefined polygon mode. Failed to create VkPipeline.");
        
    if (info->rasterizationState.frontFace == rhi::FrontFace::UNDEFINED)
        FE_LOG(LogVulkanRHI, FATAL, "VulkanPipeline::create_graphics_pipeline(): Undefined front face. Failed to create VkPipeline.");

    VkPipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.polygonMode = get_polygon_mode(info->rasterizationState.polygonMode);
    rasterizationState.lineWidth = 1.0f;
    rasterizationState.cullMode = get_cull_mode(info->rasterizationState.cullMode);
    rasterizationState.frontFace = get_front_face(info->rasterizationState.frontFace);
    rasterizationState.depthBiasEnable = info->rasterizationState.isBiasEnabled ? VK_TRUE : VK_FALSE;
    rasterizationState.depthBiasConstantFactor = 0.0f;
    rasterizationState.depthBiasClamp = 0.0f;
    rasterizationState.depthBiasSlopeFactor = 0.0f;

    if (info->multisampleState.sampleCount == rhi::SampleCount::UNDEFINED)
        FE_LOG(LogVulkanRHI, FATAL, "create_graphics_pipeline(): Undefined sample count. Failed to create VkPipeline.");

    VkPipelineMultisampleStateCreateInfo multisampleState{};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.sampleShadingEnable = info->multisampleState.isEnabled ? VK_TRUE : VK_FALSE;
    multisampleState.rasterizationSamples = get_sample_count(info->multisampleState.sampleCount);
    multisampleState.minSampleShading = 1.0f;
    multisampleState.pSampleMask = nullptr;
    multisampleState.alphaToCoverageEnable = info->multisampleState.isEnabled ? VK_TRUE : VK_FALSE;
    multisampleState.alphaToOneEnable = info->multisampleState.isEnabled ? VK_TRUE : VK_FALSE;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    for (auto& desc : info->bindingDescriptions)
    {
        VkVertexInputBindingDescription description;
        description.binding = desc.binding;
        description.stride = desc.stride;
        description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions.push_back(description);
    }

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    for (auto& desc : info->attributeDescriptions)
    {
        if (desc.format == rhi::Format::UNDEFINED)
            FE_LOG(LogVulkanRHI, FATAL, "create_graphics_pipeline(): Undefined format. Failed to create VkPipeline.");
        
        VkVertexInputAttributeDescription description;
        description.binding = desc.binding;
        description.format = get_format(desc.format);
        description.location = desc.location;
        description.offset = desc.offset;
        attributeDescriptions.push_back(description);
    }

    VkPipelineVertexInputStateCreateInfo inputState{};
    inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    inputState.pVertexBindingDescriptions = !bindingDescriptions.empty() ? bindingDescriptions.data() : nullptr;
    inputState.vertexBindingDescriptionCount = bindingDescriptions.size();
    inputState.pVertexAttributeDescriptions = !attributeDescriptions.empty() ? attributeDescriptions.data() : nullptr;
    inputState.vertexAttributeDescriptionCount = attributeDescriptions.size();

    std::vector<VkDynamicState> dynamicStates;
    dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
    if (info->rasterizationState.isBiasEnabled)
        dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    for (auto& attach : info->colorBlendState.colorBlendAttachments)
    {
        VkPipelineColorBlendAttachmentState attachmentState{};
        attachmentState.colorWriteMask = (VkColorComponentFlags)attach.colorWriteMask;
        if (attach.isBlendEnabled)
        {
            if (attach.srcColorBlendFactor == rhi::BlendFactor::UNDEFINED)
                FE_LOG(LogVulkanRHI, FATAL, "create_graphics_pipeline(): Undefined src color blend factor. Failed to create VkPipeline.");
            
            if (attach.dstColorBlendFactor == rhi::BlendFactor::UNDEFINED)
                FE_LOG(LogVulkanRHI, FATAL, "create_graphics_pipeline(): Undefined dst color blend factor. Failed to create VkPipeline.");
            
            if (attach.srcAlphaBlendFactor == rhi::BlendFactor::UNDEFINED)
                FE_LOG(LogVulkanRHI, FATAL, "create_graphics_pipeline(): Undefined src alpha blend factor. Failed to create VkPipeline.");
            
            if (attach.dstAlphaBlendFactor == rhi::BlendFactor::UNDEFINED)
                FE_LOG(LogVulkanRHI, FATAL, "create_graphics_pipeline(): Undefined dst alpha blend factor. Failed to create VkPipeline.");
            
            if (attach.colorBlendOp == rhi::BlendOp::UNDEFINED)
                FE_LOG(LogVulkanRHI, FATAL, "create_graphics_pipeline(): Undefined color blend op. Failed to create VkPipeline.");
            
            if (attach.alphaBlendOp == rhi::BlendOp::UNDEFINED)
                FE_LOG(LogVulkanRHI, FATAL, "create_graphics_pipeline(): Undefined alpha blend op. Failed to create VkPipeline.");
            
            attachmentState.blendEnable = VK_TRUE;
            attachmentState.srcColorBlendFactor = get_blend_factor(attach.srcColorBlendFactor);
            attachmentState.dstColorBlendFactor = get_blend_factor(attach.dstColorBlendFactor);
            attachmentState.srcAlphaBlendFactor = get_blend_factor(attach.srcAlphaBlendFactor);
            attachmentState.dstAlphaBlendFactor = get_blend_factor(attach.dstAlphaBlendFactor);
            attachmentState.colorBlendOp = get_blend_op(attach.colorBlendOp);
            attachmentState.alphaBlendOp = get_blend_op(attach.alphaBlendOp);
        }
        attachmentState.blendEnable = VK_FALSE;
        colorBlendAttachments.push_back(attachmentState);
    }

    VkPipelineColorBlendStateCreateInfo colorBlendState{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.attachmentCount = colorBlendAttachments.size();
    colorBlendState.pAttachments = colorBlendAttachments.data();
    colorBlendState.logicOpEnable = info->colorBlendState.isLogicOpEnabled ? VK_TRUE : VK_FALSE;
    colorBlendState.logicOp = get_logic_op(info->colorBlendState.logicOp);
    colorBlendState.blendConstants[0] = 1.0f;
    colorBlendState.blendConstants[1] = 1.0f;
    colorBlendState.blendConstants[2] = 1.0f;
    colorBlendState.blendConstants[3] = 1.0f;
    
    std::vector<VkPipelineShaderStageCreateInfo> pipelineStages;
    pipelineStages.reserve(info->shaderStages.size());
    for (Shader* shader : info->shaderStages)
    {
        if (shader->type == rhi::ShaderType::UNDEFINED)
            FE_LOG(LogVulkanRHI, FATAL, "create_graphics_pipeline(): Shader type is undefined.");
        
        VkPipelineShaderStageCreateInfo& shaderStage = pipelineStages.emplace_back();
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.pNext = nullptr;
        shaderStage.flags = 0;
        shaderStage.module = shader->vk().shader;
        shaderStage.stage = (VkShaderStageFlagBits)get_shader_stage(shader->type);
        shaderStage.pName = "main";
        shaderStage.pSpecializationInfo = nullptr;
    }

    if (info->depthStencilState.compareOp == rhi::CompareOp::UNDEFINED)
        FE_LOG(LogVulkanRHI, FATAL, "create_graphics_pipeline(): Undefined compare op, depth. Failed to create VkPipeline.");
    
    VkPipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = info->depthStencilState.isDepthTestEnabled ? VK_TRUE : VK_FALSE;
    depthStencilState.depthWriteEnable = info->depthStencilState.isDepthWriteEnabled ? VK_TRUE : VK_FALSE;
    depthStencilState.depthCompareOp = get_compare_op(info->depthStencilState.compareOp);
    depthStencilState.minDepthBounds = 0.0f;
    depthStencilState.maxDepthBounds = 1.0f;
    depthStencilState.stencilTestEnable = VK_FALSE;
    std::vector<rhi::StencilOpState> stencilOps = { info->depthStencilState.backStencil, info->depthStencilState.frontStencil };
    if (info->depthStencilState.isStencilTestEnabled)
    {
        depthStencilState.stencilTestEnable = VK_TRUE;

        std::vector<VkStencilOpState> vkStencilStates;
        for (auto& stencilState : stencilOps)
        {
            if (stencilState.compareOp == rhi::CompareOp::UNDEFINED)
                FE_LOG(LogVulkanRHI, FATAL, "create_graphics_pipeline(): Undefined compare op, stencil. Failed to create VkPipeline.");
            
            if (stencilState.failOp == rhi::StencilOp::UNDEFINED)
                FE_LOG(LogVulkanRHI, FATAL, "create_graphics_pipeline(): Undefined fail op stencil. Failed to create VkPipeline.");
            
            if (stencilState.passOp == rhi::StencilOp::UNDEFINED)
                FE_LOG(LogVulkanRHI, FATAL, "VulkanPipeline::create_graphics_pipeline(): Undefined pass op stencil. Failed to create VkPipeline.");
            
            if (stencilState.depthFailOp == rhi::StencilOp::UNDEFINED)
                FE_LOG(LogVulkanRHI, FATAL, "VulkanPipeline::create_graphics_pipeline(): Undefined depth fail op stencil. Failed to create VkPipeline.");
            
            VkStencilOpState stencilOp{};
            stencilOp.failOp = get_stencil_op(stencilState.failOp);
            stencilOp.passOp = get_stencil_op(stencilState.passOp);
            stencilOp.depthFailOp = get_stencil_op(stencilState.depthFailOp);
            stencilOp.compareOp = get_compare_op(stencilState.compareOp);
            stencilOp.compareMask = stencilState.compareMask;
            stencilOp.writeMask = stencilState.writeMask;
            stencilOp.reference = stencilState.reference;
            vkStencilStates.push_back(stencilOp);
        }
    }

    const PipelineLayoutCache::PipelineLayout& layout = g_pipelineLayoutCache.find_or_add_layout(info->shaderStages);

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.renderPass = VK_NULL_HANDLE;
    pipelineInfo.layout = layout.layout;
    pipelineInfo.subpass = 0;
    pipelineInfo.stageCount = pipelineStages.size();
    pipelineInfo.pStages = pipelineStages.data();
    pipelineInfo.pVertexInputState = &inputState;
    pipelineInfo.pInputAssemblyState = &assemblyState;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizationState;
    pipelineInfo.pMultisampleState = &multisampleState;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.pColorBlendState = &colorBlendState;
    pipelineInfo.pDepthStencilState = &depthStencilState;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    
    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingCreateInfo.viewMask = 0;
    //pipelineRenderingCreateInfo.colorAttachmentCount = info->colorAttachmentFormats.size();
    
    std::vector<VkFormat> colorAttachFormats;
    for (auto& rhiFormat : info->colorAttachmentFormats)
        colorAttachFormats.push_back(get_format(rhiFormat));
    
    pipelineRenderingCreateInfo.pColorAttachmentFormats = colorAttachFormats.size() ? colorAttachFormats.data() : nullptr;
    pipelineRenderingCreateInfo.colorAttachmentCount = colorAttachFormats.size();
    if (info->depthFormat != rhi::Format::UNDEFINED)
    {
        pipelineRenderingCreateInfo.depthAttachmentFormat = get_format(info->depthFormat);
        if (support_stencil(info->depthFormat))
            pipelineRenderingCreateInfo.stencilAttachmentFormat = get_format(info->depthFormat);
    }
    
    pipelineInfo.pNext = &pipelineRenderingCreateInfo;

    Pipeline* pipelinePtr = g_allocator.pipelineAllocator.allocate();
    FE_CHECK(pipelinePtr);
    pipelinePtr->init_vk();

    pipelinePtr->type = PipelineType::GRAPHICS;
    pipelinePtr->vk().layoutHash = layout.layoutHash;
    
    VK_CHECK(vkCreateGraphicsPipelines(g_device.device, nullptr, 1, &pipelineInfo, nullptr, &pipelinePtr->vk().pipeline));

    *pipeline = pipelinePtr;
}

void create_compute_pipelines(const std::vector<ComputePipelineInfo>& infos, std::vector<Pipeline*>& outPipelines)
{
    outPipelines.reserve(outPipelines.size() + infos.size());

    for (const ComputePipelineInfo& info : infos)
    {
        create_compute_pipeline(&outPipelines.emplace_back(), &info);
    }
}

void create_compute_pipeline(Pipeline** pipeline, const ComputePipelineInfo* info)
{
    FE_CHECK(pipeline);
    FE_CHECK(info);

    VkPipelineShaderStageCreateInfo shaderStage{};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.module = info->shaderStage->vk().shader;
    shaderStage.stage = (VkShaderStageFlagBits)get_shader_stage(info->shaderStage->type);
    shaderStage.pName = "main";

    const PipelineLayoutCache::PipelineLayout& layout = g_pipelineLayoutCache.find_or_add_layout(info->shaderStage);
    VkComputePipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.layout = layout.layout;
    createInfo.stage = shaderStage;

    Pipeline* pipelinePtr = g_allocator.pipelineAllocator.allocate();
    FE_CHECK(pipelinePtr);
    pipelinePtr->init_vk();

    pipelinePtr->type = PipelineType::COMPUTE;
    pipelinePtr->vk().layoutHash = layout.layoutHash;

    VK_CHECK(vkCreateComputePipelines(g_device.device, nullptr, 1, &createInfo, nullptr, &pipelinePtr->vk().pipeline));

    *pipeline = pipelinePtr;
}

void create_ray_tracing_pipeline(Pipeline** pipeline, const RayTracingPipelineInfo* info)
{
    FE_CHECK(pipeline);
    FE_CHECK(info);

    VkRayTracingPipelineCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    
    std::vector<VkPipelineShaderStageCreateInfo> pipelineStages;
    pipelineStages.reserve(info->shaderLibraries.size());
    std::vector<Shader*> shaderPtrs;
    shaderPtrs.reserve(info->shaderLibraries.size());

    for (const ShaderLibrary& shaderLibrary : info->shaderLibraries)
    {
        if (shaderLibrary.type == rhi::ShaderType::UNDEFINED)
            FE_LOG(LogVulkanRHI, FATAL, "create_graphics_pipeline(): Shader type is undefined.");

        FE_CHECK(shaderLibrary.shader);
        shaderPtrs.push_back(shaderLibrary.shader);
        
        VkPipelineShaderStageCreateInfo& shaderStage = pipelineStages.emplace_back();
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.pNext = nullptr;
        shaderStage.flags = 0;
        shaderStage.module = shaderLibrary.shader->vk().shader;
        shaderStage.stage = (VkShaderStageFlagBits)get_shader_stage(shaderLibrary.type);
        shaderStage.pName = shaderLibrary.entryPoint.c_str();
        shaderStage.pSpecializationInfo = nullptr;
    }

    createInfo.stageCount = pipelineStages.size();
    createInfo.pStages = pipelineStages.data();

    std::vector<VkRayTracingShaderGroupCreateInfoKHR> hitGroupInfos;
    hitGroupInfos.reserve(info->shaderHitGroups.size());

    for (const ShaderHitGroup& hitGroup : info->shaderHitGroups)
    {
        VkRayTracingShaderGroupCreateInfoKHR& hitGroupInfo = hitGroupInfos.emplace_back();
        hitGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        hitGroupInfo.pNext = nullptr;
        hitGroupInfo.pShaderGroupCaptureReplayHandle = nullptr;

        switch (hitGroup.type)
        {
        case ShaderHitGroup::GENERAL:
            hitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            break;
        case ShaderHitGroup::PROCEDURAL:
            hitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
            break;
        case ShaderHitGroup::TRIANGLES:
            hitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            break;
        }

        hitGroupInfo.generalShader = hitGroup.generalShader;
        hitGroupInfo.closestHitShader = hitGroup.closestHitShader;
        hitGroupInfo.anyHitShader = hitGroup.anyHitShader;
        hitGroupInfo.intersectionShader = hitGroup.intersectionShader;
    }

    createInfo.groupCount = hitGroupInfos.size();
    createInfo.pGroups = hitGroupInfos.data();

    createInfo.maxPipelineRayRecursionDepth = info->maxTraceDepthRecursion;

    const PipelineLayoutCache::PipelineLayout& layout = g_pipelineLayoutCache.find_or_add_layout(shaderPtrs);
    createInfo.layout = layout.layout;

    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = 0;

    Pipeline* pipelinePtr = g_allocator.pipelineAllocator.allocate();
    FE_CHECK(pipelinePtr);
    pipelinePtr->init_vk();

    pipelinePtr->type = PipelineType::RAY_TRACING;
    pipelinePtr->vk().layoutHash = layout.layoutHash;

    VK_CHECK(vkCreateRayTracingPipelinesKHR(g_device.device, VK_NULL_HANDLE, nullptr, 1, &createInfo, nullptr, &pipelinePtr->vk().pipeline));

    *pipeline = pipelinePtr;
}

void destroy_pipeline(Pipeline* pipeline)
{
    if (!pipeline)
        return;

    if (pipeline->vk().pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(g_device.device, pipeline->vk().pipeline, nullptr);

    g_allocator.pipelineAllocator.free(pipeline);
}

void create_acceleration_structure(AccelerationStructure** accelerationStructure, AccelerationStructureInfo* info)
{
    FE_CHECK(accelerationStructure);
    FE_CHECK(info);

    AccelerationStructure* accelerationStructurePtr = g_allocator.accelerationStructureAllocator.allocate();
    FE_CHECK(accelerationStructurePtr);
    accelerationStructurePtr->init_vk();
    accelerationStructurePtr->vk().accelerationStructure = VK_NULL_HANDLE;

    accelerationStructurePtr->info = *info;

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
    std::vector<VkAccelerationStructureGeometryKHR> geometries;
    std::vector<uint32> primitivesCount;

    fill_acceleration_structure_build_geometry_info(
        accelerationStructurePtr, 
        nullptr,
        &buildInfo,
        &geometries,
        &primitivesCount,
        nullptr
    );

    VkAccelerationStructureBuildSizesInfoKHR sizesInfo{};
    sizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

    vkGetAccelerationStructureBuildSizesKHR(
        g_device.device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &buildInfo,
        primitivesCount.data(),
        &sizesInfo
    );

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizesInfo.accelerationStructureSize + std::max(sizesInfo.buildScratchSize, sizesInfo.updateScratchSize);
    bufferInfo.usage = VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
        | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
        | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    if (g_device.queueFamilies.size() > 1)
    {
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        bufferInfo.queueFamilyIndexCount = g_device.queueFamilies.size();
        bufferInfo.pQueueFamilyIndices = g_device.queueFamilies.data();
    }
    else
    {
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VK_CHECK(vmaCreateBuffer(
        g_allocator.gpuAllocator, 
        &bufferInfo, 
        &allocInfo, 
        &accelerationStructurePtr->vk().buffer, 
        &accelerationStructurePtr->vk().allocation, 
        nullptr
    ));

    VkAccelerationStructureCreateInfoKHR asCreateInfo{};
    asCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    asCreateInfo.type = buildInfo.type;
    asCreateInfo.buffer = accelerationStructurePtr->vk().buffer;
    asCreateInfo.size = sizesInfo.accelerationStructureSize;

    VK_CHECK(vkCreateAccelerationStructureKHR(
        g_device.device,
        &asCreateInfo,
        nullptr,
        &accelerationStructurePtr->vk().accelerationStructure
    ));

    accelerationStructurePtr->vk().accelerationStructureAddress = 
        get_device_address(accelerationStructurePtr->vk().accelerationStructure);

    accelerationStructurePtr->vk().scratchAddress = get_device_address(accelerationStructurePtr->vk().buffer)
        + sizesInfo.accelerationStructureSize;

    if (info->type == AccelerationStructureInfo::TOP_LEVEL)
        g_descriptorHeap.allocate_descriptor(accelerationStructurePtr);

    *accelerationStructure = accelerationStructurePtr;
}

void destroy_acceleration_structure(AccelerationStructure* accelerationStructure)
{
    if (!accelerationStructure) 
        return;

    if (accelerationStructure->info.type == AccelerationStructureInfo::TOP_LEVEL)
        g_descriptorHeap.free_descriptor(accelerationStructure);

    if (accelerationStructure->vk().buffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(g_allocator.gpuAllocator, accelerationStructure->vk().buffer, accelerationStructure->vk().allocation);

    if (accelerationStructure->vk().accelerationStructure != VK_NULL_HANDLE)
        vkDestroyAccelerationStructureKHR(g_device.device, accelerationStructure->vk().accelerationStructure, nullptr);

    g_allocator.accelerationStructureAllocator.free(accelerationStructure);
}

void write_top_level_acceleration_structure_instance(TLAS::Instance* instance, void* dst)
{
    FE_CHECK(instance);
    FE_CHECK(instance->blas);

    VkAccelerationStructureInstanceKHR vkInstance{};
    vkInstance.transform = *(VkTransformMatrixKHR*)&instance->transform;
    vkInstance.instanceCustomIndex = instance->instanceID;
    vkInstance.mask = instance->instanceMask;
    vkInstance.instanceShaderBindingTableRecordOffset = instance->instanceContributionToHitGroupIndex;
    vkInstance.flags = 0;

    if (has_flag(instance->flags, TLAS::Instance::Flags::TRIANGLE_CULL_DISABLE))
        vkInstance.flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    if (has_flag(instance->flags, TLAS::Instance::Flags::TRIANGLE_FRONT_COUNTERCLOCKWISE))
        vkInstance.flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR;
    if (has_flag(instance->flags, TLAS::Instance::Flags::FORCE_OPAQUE))
        vkInstance.flags |= VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR;
    if (has_flag(instance->flags, TLAS::Instance::Flags::FORCE_NON_OPAQUE))
        vkInstance.flags |= VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR;

    vkInstance.accelerationStructureReference = instance->blas->vk().accelerationStructureAddress;

    std::memcpy(dst, &vkInstance, sizeof(VkAccelerationStructureInstanceKHR));
}

void write_shader_identifier(Pipeline* pipeline, uint32 groupIndex, void* dst)
{
    FE_CHECK(pipeline->type == rhi::PipelineType::RAY_TRACING);

    vkGetRayTracingShaderGroupHandlesKHR(
        g_device.device, 
        pipeline->vk().pipeline, 
        groupIndex, 
        1,
        get_shader_identifier_size(),
        dst 
    );
}

void bind_uniform_buffer(Buffer* buffer, uint32 frameIndex, uint32 slot, uint32 size, uint32 offset)
{
    FE_CHECK(buffer);

    if (!has_flag(buffer->bufferUsage, rhi::ResourceUsage::UNIFORM_BUFFER))
        FE_LOG(LogVulkanRHI, FATAL, "bind_uniform_buffer(): Can't bind buffer without ResourceUsage::UNIFORM_BUFFER");

    if (!size)
        size = buffer->size;

    g_descriptorHeap.allocate_uniform_buffer(buffer, size, offset, slot, frameIndex);
}

void create_command_pool(CommandPool** cmdPool, const CommandPoolInfo* info)
{
    FE_CHECK(cmdPool);
    FE_CHECK(info);

    CommandPool* cmdPoolPtr = g_allocator.cmdPoolAllocator.allocate();
    FE_CHECK(cmdPoolPtr);
    cmdPoolPtr->init_vk();

    cmdPoolPtr->queueType = info->queueType;
    
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.queueFamilyIndex = g_device.get_queue(info->queueType).family;

    VK_CHECK(vkCreateCommandPool(g_device.device, &createInfo, nullptr, &cmdPoolPtr->vk().cmdPool));

    *cmdPool = cmdPoolPtr;
}

void destroy_command_pool(CommandPool* cmdPool)
{
    if (!cmdPool)
        return;

    if (cmdPool->vk().cmdPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(g_device.device, cmdPool->vk().cmdPool, nullptr);

    g_allocator.cmdPoolAllocator.free(cmdPool);
}

void create_command_buffer(CommandBuffer** cmd, const CommandBufferInfo* info)
{
    FE_CHECK(cmd);
    FE_CHECK(info);

    CommandBuffer* cmdPtr = g_allocator.cmdBufferAllocator.allocate();
    FE_CHECK(cmdPtr);
    cmdPtr->init_vk();

    cmdPtr->cmdPool = info->cmdPool;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = info->cmdPool->vk().cmdPool;
    VK_CHECK(vkAllocateCommandBuffers(g_device.device, &allocInfo, &cmdPtr->vk().cmdBuffer));

    *cmd = cmdPtr;
}

void destroy_command_buffer(CommandBuffer* cmd)
{
    if (!cmd)
        return;

    if (cmd->vk().cmdBuffer != VK_NULL_HANDLE)
        vkFreeCommandBuffers(g_device.device, cmd->cmdPool->vk().cmdPool, 1, &cmd->vk().cmdBuffer);

    g_allocator.cmdBufferAllocator.free(cmd);
}

void begin_command_buffer(CommandBuffer* cmd)
{
    FE_CHECK(cmd);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pInheritanceInfo = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd->vk().cmdBuffer, &beginInfo);
}

void end_command_buffer(CommandBuffer* cmd)
{
    FE_CHECK(cmd);

    if (cmd->vk().cmdBuffer != VK_NULL_HANDLE)
        vkEndCommandBuffer(cmd->vk().cmdBuffer);
}

void reset_command_pool(CommandPool* cmdPool)
{
    FE_CHECK(cmdPool);

    if (cmdPool->vk().cmdPool != VK_NULL_HANDLE)
        vkResetCommandPool(g_device.device, cmdPool->vk().cmdPool, 0);
}

void create_semaphore(Semaphore** semaphore)
{
    FE_CHECK(semaphore);

    Semaphore* semaphorePtr = g_allocator.semaphoreAllocator.allocate();
    FE_CHECK(semaphorePtr);
    semaphorePtr->init_vk();

    VkSemaphoreCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.flags = 0;
    VK_CHECK(vkCreateSemaphore(g_device.device, &info, nullptr, &semaphorePtr->vk().semaphore));

    *semaphore = semaphorePtr;
}

void destroy_semaphore(Semaphore* semaphore)
{
    if (!semaphore)
        return;

    if (semaphore->vk().semaphore != VK_NULL_HANDLE)
        vkDestroySemaphore(g_device.device, semaphore->vk().semaphore, nullptr);
}

void create_fence(Fence** fence)
{
    FE_CHECK(fence);

    Fence* fencePtr = g_allocator.fenceAllocator.allocate();
    FE_CHECK(fencePtr);
    fencePtr->init_vk();

    VkFenceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VK_CHECK(vkCreateFence(g_device.device, &info, nullptr, &fencePtr->vk().fence));

    *fence = fencePtr;
}

void destroy_fence(Fence* fence)
{
    if (!fence)
        return;

    if (fence->vk().fence != VK_NULL_HANDLE)
        vkDestroyFence(g_device.device, fence->vk().fence, nullptr);
}

void fill_buffer(CommandBuffer* cmd, Buffer* dstBuffer, uint32 dstOffset, uint32 size, uint32 data)
{
    FE_CHECK(cmd);
    FE_CHECK(dstBuffer);
    
    vkCmdFillBuffer(cmd->vk().cmdBuffer, dstBuffer->vk().buffer, dstOffset, size, data);
}

void copy_buffer(CommandBuffer* cmd, Buffer* srcBuffer, Buffer* dstBuffer, uint32 size, uint32 srcOffset, uint32 dstOffset)
{
    FE_CHECK(cmd);
    FE_CHECK(srcBuffer);
    FE_CHECK(dstBuffer);

    if (!has_flag(srcBuffer->bufferUsage, rhi::ResourceUsage::TRANSFER_SRC))
        FE_LOG(LogVulkanRHI, FATAL, "copy_buffer(): Source buffer doesn't have TRANSFER_SRC usage.");
        
    if (!has_flag(dstBuffer->bufferUsage, rhi::ResourceUsage::TRANSFER_DST))
        FE_LOG(LogVulkanRHI, FATAL, "copy_buffer(): Destination buffer doesn't have TRANSFER_DST usage.");

    VkBufferCopy2 copy{};
    copy.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
    copy.srcOffset = srcOffset;
    copy.dstOffset = dstOffset;
    copy.size = size ? size : srcBuffer->size;

    VkCopyBufferInfo2 vkBufferInfo{};
    vkBufferInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
    vkBufferInfo.srcBuffer = srcBuffer->vk().buffer;
    vkBufferInfo.dstBuffer = dstBuffer->vk().buffer;
    vkBufferInfo.regionCount = 1;
    vkBufferInfo.pRegions = &copy;
    vkCmdCopyBuffer2(cmd->vk().cmdBuffer, &vkBufferInfo);
}

void copy_texture(CommandBuffer* cmd, Texture* srcTexture, Texture* dstTexture)
{
    FE_CHECK(cmd);
    FE_CHECK(srcTexture);
    FE_CHECK(dstTexture);

    if (!has_flag(srcTexture->textureUsage, rhi::ResourceUsage::TRANSFER_SRC))
        FE_LOG(LogVulkanRHI, FATAL, "copy_texture(): Source texture doesn't have TRANSFER_SRC usage.");
        
    if (!has_flag(dstTexture->textureUsage, rhi::ResourceUsage::TRANSFER_DST))
        FE_LOG(LogVulkanRHI, FATAL, "copy_texture(): Destination texture doesn't have TRANSFER_DST usage.");

    VkImageCopy2 copy{};
    copy.sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2;
    copy.extent.width = dstTexture->width;
    copy.extent.height = dstTexture->height;
    copy.extent.depth = std::max(1u, dstTexture->depth);
    copy.srcOffset = { 0, 0, 0 };
    copy.dstOffset = { 0, 0, 0 };

    copy.srcSubresource.aspectMask = get_image_aspect(srcTexture->textureUsage);
    copy.srcSubresource.baseArrayLayer = 0;
    copy.srcSubresource.layerCount = srcTexture->layersCount;
    copy.srcSubresource.mipLevel = 0;

    copy.dstSubresource.aspectMask = get_image_aspect(dstTexture->textureUsage);
    copy.dstSubresource.baseArrayLayer = 0;
    copy.dstSubresource.layerCount = dstTexture->layersCount;
    copy.dstSubresource.mipLevel = 0;

    VkCopyImageInfo2 copyImageInfo{};
    copyImageInfo.sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2;
    copyImageInfo.srcImage = srcTexture->vk().image;
    copyImageInfo.dstImage = dstTexture->vk().image;
    copyImageInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    copyImageInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    copyImageInfo.regionCount = 1;
    copyImageInfo.pRegions = &copy;

    vkCmdCopyImage2(cmd->vk().cmdBuffer, &copyImageInfo);
}

void init_texture(CommandBuffer* cmd, Texture* dstTexture, const TextureInitInfo* initInfo)
{
    FE_CHECK(cmd);
    FE_CHECK(dstTexture);
    FE_CHECK(initInfo);
    FE_CHECK(!initInfo->mipMaps.empty());

    add_pre_transfer_image_barrier(cmd, dstTexture);

    std::vector<VkBufferImageCopy2> copyRegions;
    copyRegions.reserve(initInfo->mipMaps.size());

    uint32 width = dstTexture->width;
    uint32 height = dstTexture->height;
    uint32 depth = dstTexture->depth;
    uint32 mipLevel = 0;

    for (const MipMap& mipMap : initInfo->mipMaps)
    {
        VkBufferImageCopy2& copyRegion = copyRegions.emplace_back();
        copyRegion.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
        copyRegion.bufferOffset = mipMap.offset;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;
        copyRegion.imageExtent = { width, height, depth };
        copyRegion.imageOffset = { 0, 0, 0 };
        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = mipLevel++;
        copyRegion.imageSubresource.baseArrayLayer = mipMap.layer;
        copyRegion.imageSubresource.layerCount = 1;

        width = std::max(1u, width / 2u);
        height = std::max(1u, height / 2u);
        depth = std::max(1u, depth / 2u);
    }

    VkCopyBufferToImageInfo2 copyBufferToImageInfo{};
    copyBufferToImageInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2;
    copyBufferToImageInfo.regionCount = copyRegions.size();
    copyBufferToImageInfo.pRegions = copyRegions.data();
    copyBufferToImageInfo.srcBuffer = initInfo->buffer->vk().buffer;
    copyBufferToImageInfo.dstImage = dstTexture->vk().image;
    copyBufferToImageInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    vkCmdCopyBufferToImage2(cmd->vk().cmdBuffer, &copyBufferToImageInfo);

    add_post_transfer_image_barrier(cmd, dstTexture);
}

void copy_buffer_to_texture(CommandBuffer* cmd, Buffer* srcBuffer, Texture* dstTexture)
{
    FE_CHECK(cmd);
    FE_CHECK(srcBuffer);
    FE_CHECK(dstTexture);

    if (!has_flag(srcBuffer->bufferUsage, rhi::ResourceUsage::TRANSFER_SRC))
        FE_LOG(LogVulkanRHI, FATAL, "copy_buffer_to_texture(): Source buffer doesn't have TRANSFER_SRC usage.");
    if (!has_flag(dstTexture->textureUsage, rhi::ResourceUsage::TRANSFER_DST))
        FE_LOG(LogVulkanRHI, FATAL, "copy_buffer_to_texture(): Destination buffer doesn't have TRANSFER_DST usage.");

    add_pre_transfer_image_barrier(cmd, dstTexture);
    
    VkBufferImageCopy2 copyRegion{};
    copyRegion.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;
    copyRegion.imageExtent = {dstTexture->width, dstTexture->height, dstTexture->depth};
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = dstTexture->layersCount;

    VkCopyBufferToImageInfo2 copyBufferToImageInfo{};
    copyBufferToImageInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2;
    copyBufferToImageInfo.regionCount = 1;
    copyBufferToImageInfo.pRegions = &copyRegion;
    copyBufferToImageInfo.srcBuffer = srcBuffer->vk().buffer;
    copyBufferToImageInfo.dstImage = dstTexture->vk().image;
    copyBufferToImageInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    
    vkCmdCopyBufferToImage2(cmd->vk().cmdBuffer, &copyBufferToImageInfo);
    
    add_post_transfer_image_barrier(cmd, dstTexture);
}

void copy_texture_to_buffer(CommandBuffer* cmd, Texture* srcTexture, Buffer* dstBuffer)
{
    FE_CHECK(cmd);
    FE_CHECK(srcTexture);
    FE_CHECK(dstBuffer);

    if (!has_flag(srcTexture->textureUsage, rhi::ResourceUsage::TRANSFER_SRC))
        FE_LOG(LogVulkanRHI, FATAL, "copy_texture_to_buffer(): Source texture doesn't have TRANSFER_SRC usage.");
    if (!has_flag(dstBuffer->bufferUsage, rhi::ResourceUsage::TRANSFER_DST))
        FE_LOG(LogVulkanRHI, FATAL, "copy_texture_to_buffer(): Destination buffer doesn't have TRANSFER_DST usage.");
    
    VkCopyImageToBufferInfo2 copyImageToBufferInfo{};
    copyImageToBufferInfo.sType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2;
    copyImageToBufferInfo.srcImage = srcTexture->vk().image;
    copyImageToBufferInfo.dstBuffer = dstBuffer->vk().buffer;
    copyImageToBufferInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    copyImageToBufferInfo.regionCount = 1;

    VkBufferImageCopy2 copy{};
    copy.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
    copy.imageSubresource.aspectMask = get_image_aspect(srcTexture->textureUsage);
    copy.imageSubresource.baseArrayLayer = 0;
    copy.imageSubresource.layerCount = srcTexture->layersCount;
    const uint32_t dataStride = get_format_stride(srcTexture->format);
    uint32_t mipWidth = srcTexture->width;
    uint32_t mipHeight = srcTexture->height;
    uint32_t mipDepth = std::max(1u, srcTexture->depth);
    for (uint32_t mipLevel = 0; mipLevel < srcTexture->mipLevels; ++mipLevel)
    {
        copy.imageExtent = { mipWidth, mipHeight, mipDepth };
        copy.imageSubresource.mipLevel = mipLevel;
        
        copyImageToBufferInfo.pRegions = &copy;
        vkCmdCopyImageToBuffer2(cmd->vk().cmdBuffer, &copyImageToBufferInfo);

        copy.bufferOffset += mipWidth * mipHeight * mipDepth * dataStride;
        mipWidth = std::max(1u, mipWidth / 2);
        mipHeight = std::max(1u, mipHeight / 2);
        mipDepth = std::max(1u, mipDepth / 2);
    }
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
    FE_CHECK(cmd);
    FE_CHECK(srcTexture);
    FE_CHECK(dstTexture);

    if (!has_flag(srcTexture->textureUsage, rhi::ResourceUsage::TRANSFER_SRC))
        FE_LOG(LogVulkanRHI, FATAL, "VulkanRHI::blit_texture(): Source buffer doesn't have TRANSFER_SRC usage.");
    if (!has_flag(dstTexture->textureUsage, rhi::ResourceUsage::TRANSFER_DST))
        FE_LOG(LogVulkanRHI, FATAL, "VulkanRHI::blit_texture(): Destination buffer doesn't have TRANSFER_DST usage.");

    VkImageBlit2 imageBlit{};
    imageBlit.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
    imageBlit.srcOffsets[1].x = srcOffset[0];
    imageBlit.srcOffsets[1].y = srcOffset[1];
    imageBlit.srcOffsets[1].z = srcOffset[2];
    imageBlit.dstOffsets[1].x = dstOffset[0];
    imageBlit.dstOffsets[1].y = dstOffset[1];
    imageBlit.dstOffsets[1].z = dstOffset[2];
    
    imageBlit.srcSubresource.aspectMask = get_image_aspect(srcTexture->textureUsage);
    imageBlit.srcSubresource.layerCount = srcTexture->layersCount;
    imageBlit.srcSubresource.baseArrayLayer = srcBaseLayer;
    imageBlit.srcSubresource.mipLevel = srcMipLevel;

    imageBlit.dstSubresource.aspectMask = get_image_aspect(dstTexture->textureUsage);
    imageBlit.dstSubresource.layerCount = dstTexture->layersCount;
    imageBlit.dstSubresource.baseArrayLayer = dstBaseLayer;
    imageBlit.dstSubresource.mipLevel = dstMipLevel;

    VkBlitImageInfo2 blitImageInfo{};
    blitImageInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
    blitImageInfo.srcImage = srcTexture->vk().image;
    blitImageInfo.dstImage = dstTexture->vk().image;
    blitImageInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    blitImageInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    blitImageInfo.regionCount = 1;
    blitImageInfo.pRegions = &imageBlit;
    blitImageInfo.filter = VK_FILTER_LINEAR;
    
    vkCmdBlitImage2(cmd->vk().cmdBuffer, &blitImageInfo);
}

void set_viewports(CommandBuffer* cmd, const std::vector<Viewport>& viewports)
{
    FE_CHECK(cmd);

    constexpr uint32 maxViewportCount = 16;

	VkViewport vulkanViewports[maxViewportCount];
	if (viewports.size() > maxViewportCount)
		FE_LOG(LogVulkanRHI, FATAL, "set_viewports(): You want to set {} viewports if there cannot be more than {} viewports.", viewports.size(), maxViewportCount);

	for (uint32_t i = 0; i != viewports.size(); ++i)
	{
		const Viewport& viewport = viewports[i];
		vulkanViewports[i].x = viewport.x;
		vulkanViewports[i].y = viewport.height;
		vulkanViewports[i].width = viewport.width;
		vulkanViewports[i].height = -(int32)viewport.height;
		vulkanViewports[i].minDepth = viewport.minDepth;
		vulkanViewports[i].maxDepth = viewport.maxDepth;
	}

	vkCmdSetViewport(cmd->vk().cmdBuffer, 0, viewports.size(), vulkanViewports);
}

void set_scissors(CommandBuffer* cmd, const std::vector<Scissor>& scissors)
{
    FE_CHECK(cmd);

    constexpr uint32 maxScissorCount = 16;

    VkRect2D vulkanScissors[maxScissorCount];
    if (scissors.size() > maxScissorCount)
        FE_LOG(LogVulkanRHI, FATAL, "set_scissors(): You want to set {} scissors if there cannot be more than {} scissors.", scissors.size(), maxScissorCount);

    for (uint32_t i = 0; i != scissors.size(); ++i)
    {
        const Scissor& scissor = scissors[i];
        vulkanScissors[i].extent.width = abs(scissor.right - scissor.left);
        vulkanScissors[i].extent.height = abs(scissor.bottom - scissor.top);
        vulkanScissors[i].offset.x = std::max(0, scissor.left);
        vulkanScissors[i].offset.y = std::max(0, scissor.top);
    }

    vkCmdSetScissor(cmd->vk().cmdBuffer, 0, scissors.size(), vulkanScissors);
}

void push_constants(CommandBuffer* cmd, Pipeline* pipeline, void* data)
{
    FE_CHECK(cmd);
    FE_CHECK(pipeline);
    FE_CHECK(data);

    if (auto pipelineLayout = g_pipelineLayoutCache.find_layout(pipeline->vk().layoutHash))
    {
        pipelineLayout->push_constants(cmd->vk().cmdBuffer, data);
    }
}

void bind_vertex_buffer(CommandBuffer* cmd, Buffer* buffer)
{
    FE_CHECK(cmd);
    FE_CHECK(buffer);

    if (!has_flag(buffer->bufferUsage, rhi::ResourceUsage::VERTEX_BUFFER))
        FE_LOG(LogVulkanRHI, FATAL, "bind_vertex_buffer(): Buffer wasn't created with VERTEX_BUFFER usage.");

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd->vk().cmdBuffer, 0, 1, &buffer->vk().buffer, &offset);
}

void bind_index_buffer(CommandBuffer* cmd, Buffer* buffer, uint64 offset)
{
    FE_CHECK(cmd);
    FE_CHECK(buffer);

    if (!has_flag(buffer->bufferUsage, rhi::ResourceUsage::INDEX_BUFFER))
        FE_LOG(LogVulkanRHI, FATAL, "bind_index_buffer(): Buffer wasn't created with INDEX_BUFFER usage.");

	vkCmdBindIndexBuffer(cmd->vk().cmdBuffer, buffer->vk().buffer, offset, VK_INDEX_TYPE_UINT32);
}

void bind_pipeline(CommandBuffer* cmd, Pipeline* pipeline)
{
    FE_CHECK(cmd);
    FE_CHECK(pipeline);

    VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;

    switch (pipeline->type)
    {
    case PipelineType::GRAPHICS:
        bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        break;
    case PipelineType::COMPUTE:
        bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
        break;
    case PipelineType::RAY_TRACING:
        bindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
        break;
    default:
        FE_LOG(LogVulkanRHI, FATAL, "bind_pipeline(): Pipeline type is UNDEFINED.");
        break;
    }
    
    vkCmdBindPipeline(cmd->vk().cmdBuffer, bindPoint, pipeline->vk().pipeline);

    if (auto pipelineLayout = g_pipelineLayoutCache.find_layout(pipeline->vk().layoutHash))
    {
        pipelineLayout->bind_descriptor_sets(cmd->vk().cmdBuffer, g_frameIndex, bindPoint);
    }
}

void build_acceleration_structure(CommandBuffer* cmd, const AccelerationStructure* dst, const AccelerationStructure* src)
{
    FE_CHECK(cmd);
    FE_CHECK(dst);

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
    std::vector<VkAccelerationStructureGeometryKHR> geometries;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> ranges;

    fill_acceleration_structure_build_geometry_info(dst, src, &buildInfo, &geometries, nullptr, &ranges);

    VkAccelerationStructureBuildRangeInfoKHR* pRangeInfo = ranges.data();

    vkCmdBuildAccelerationStructuresKHR(cmd->vk().cmdBuffer, 1, &buildInfo, &pRangeInfo);
}

void begin_rendering(CommandBuffer* cmd, RenderingBeginInfo* beginInfo)
{
    FE_CHECK(cmd);
    FE_CHECK(beginInfo);

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;

    if (has_flag(beginInfo->flags, rhi::RenderingBeginInfoFlags::RESUMING))
    {
        renderingInfo.flags |= VK_RENDERING_RESUMING_BIT;
    }
    if (has_flag(beginInfo->flags, rhi::RenderingBeginInfoFlags::SUSPENDING))
    {
        renderingInfo.flags |= VK_RENDERING_SUSPENDING_BIT;
    }
    
    renderingInfo.layerCount = 1;
    if (beginInfo->multiviewInfo.isEnabled)
    {
        renderingInfo.viewMask = (1 << beginInfo->multiviewInfo.viewCount) - 1;
    }
    
    renderingInfo.renderArea.offset.x = 0;
    renderingInfo.renderArea.offset.y = 0;

    switch (beginInfo->type)
    {
    case RenderingBeginInfo::OFFSCREEN_PASS:
    {
        if (beginInfo->offscreenPass.renderTargets.empty())
            FE_LOG(LogVulkanRHI, FATAL, "begin_rendering(): No render targets.");

            const RenderingBeginInfo::OffscreenPass& offscreenPass = beginInfo->offscreenPass;

        TextureView* renderTarget = offscreenPass.renderTargets[0].target;
        FE_CHECK(renderTarget);
        renderingInfo.renderArea.extent = {renderTarget->texture->width, renderTarget->texture->height};

        renderingInfo.colorAttachmentCount = 0;
        renderingInfo.pColorAttachments = nullptr;
        renderingInfo.pDepthAttachment = nullptr;
        renderingInfo.pStencilAttachment = nullptr;

        std::vector<VkRenderingAttachmentInfo> colorAttachments;
        VkRenderingAttachmentInfo depthAttachment{};
        VkRenderingAttachmentInfo stencilAttachment{};

        for (auto& renderTarget : offscreenPass.renderTargets)
        {
            const Texture* texture = renderTarget.target->texture;

            if (has_flag(texture->textureUsage, rhi::ResourceUsage::COLOR_ATTACHMENT))
            {
                VkRenderingAttachmentInfo& attachmentInfo = colorAttachments.emplace_back();
                attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                attachmentInfo.pNext = nullptr;
                attachmentInfo.storeOp = get_attach_store_op(renderTarget.storeOp);
                attachmentInfo.loadOp = get_attach_load_op(renderTarget.loadOp);
                attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachmentInfo.imageView = renderTarget.target->vk().imageView;
                attachmentInfo.clearValue.color = { { 
                    renderTarget.clearValue.color[0],
                    renderTarget.clearValue.color[1],
                    renderTarget.clearValue.color[2],
                    renderTarget.clearValue.color[3] 
                } };
            }
            else if (has_flag(texture->textureUsage, rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT))
            {
                if (texture->format != rhi::Format::S8_UINT)
                {
                    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                    depthAttachment.imageView = renderTarget.target->vk().imageView;
                    depthAttachment.loadOp = get_attach_load_op(renderTarget.loadOp);
                    depthAttachment.storeOp = get_attach_store_op(renderTarget.storeOp);
                    depthAttachment.clearValue.depthStencil.depth = renderTarget.clearValue.depthStencil.depth;

                    if (texture->format != rhi::Format::D16_UNORM && texture->format != rhi::Format::D32_SFLOAT)
                    {
                        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        depthAttachment.clearValue.depthStencil.stencil = renderTarget.clearValue.depthStencil.stencil;
                    }
                    else
                    {
                        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                    }

                    renderingInfo.pDepthAttachment = &depthAttachment;
                }
                else
                {
                    stencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                    stencilAttachment.imageView = renderTarget.target->vk().imageView;
                    stencilAttachment.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                    stencilAttachment.loadOp = get_attach_load_op(renderTarget.loadOp);
                    stencilAttachment.storeOp = get_attach_store_op(renderTarget.storeOp);
                    stencilAttachment.clearValue.depthStencil.stencil = renderTarget.clearValue.depthStencil.stencil;
                    renderingInfo.pStencilAttachment = &stencilAttachment;
                }
            }
        }
        
        if (!colorAttachments.empty())
        {
            renderingInfo.colorAttachmentCount = colorAttachments.size();
            renderingInfo.pColorAttachments = colorAttachments.data();
        }
        vkCmdBeginRendering(cmd->vk().cmdBuffer, &renderingInfo);
        break;
    }
    case RenderingBeginInfo::SWAP_CHAIN_PASS:
    {
        const RenderingBeginInfo::SwapChainPass& swapChainPass = beginInfo->swapChainPass;
        SwapChain* swapChain = swapChainPass.swapChain;

        FE_CHECK(swapChain);
        FE_CHECK(swapChain->vk().swapChain != VK_NULL_HANDLE);

        VkImageMemoryBarrier2 imageBarrier{};
        imageBarrier.image = swapChain->vk().images[swapChain->vk().imageIndex];
        imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        imageBarrier.srcAccessMask = VK_ACCESS_2_NONE;
        imageBarrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
        execute_image_barrier(cmd, imageBarrier);

        renderingInfo.pDepthAttachment = nullptr;
        renderingInfo.pStencilAttachment = nullptr;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.renderArea.extent.width = swapChain->window->get_info().width;
        renderingInfo.renderArea.extent.height = swapChain->window->get_info().height;
        
        VkRenderingAttachmentInfo attachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentInfo.imageView = swapChain->vk().imageViews[swapChain->vk().imageIndex];
        attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentInfo.clearValue.color = { { 
            swapChainPass.clearValues.color[0],
            swapChainPass.clearValues.color[1],
            swapChainPass.clearValues.color[2],
            swapChainPass.clearValues.color[3]
        } };

        FE_CHECK(attachmentInfo.imageView != VK_NULL_HANDLE);
        
        renderingInfo.pColorAttachments = &attachmentInfo;
        vkCmdBeginRendering(cmd->vk().cmdBuffer, &renderingInfo);

        break;
    }
    }
}

void end_rendering(CommandBuffer* cmd, SwapChain* swapChain)
{
    FE_CHECK(cmd);

    vkCmdEndRendering(cmd->vk().cmdBuffer);

    if (swapChain)
    {
        VkImageMemoryBarrier2 imageBarrier{};
        imageBarrier.image = swapChain->vk().images[swapChain->vk().imageIndex];
        imageBarrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imageBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        imageBarrier.dstAccessMask = VK_ACCESS_2_NONE;

        execute_image_barrier(cmd, imageBarrier);
    }
}

void draw(CommandBuffer* cmd, uint64 vertexCount)
{
    FE_CHECK(cmd);

    vkCmdDraw(cmd->vk().cmdBuffer, vertexCount, 1, 0, 0);
}

void draw_indexed(CommandBuffer* cmd, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance)
{
    FE_CHECK(cmd);
    vkCmdDrawIndexed(cmd->vk().cmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void draw_indirect(CommandBuffer* cmd, Buffer* buffer, uint32 offset, uint32 drawCount, uint32 stride)
{
    FE_CHECK(cmd);
    FE_CHECK(buffer);

    vkCmdDrawIndirect(cmd->vk().cmdBuffer, buffer->vk().buffer, offset, drawCount, stride);
}

void draw_indexed_indirect(CommandBuffer* cmd, Buffer* buffer, uint32 offset, uint32 drawCount, uint32 stride)
{
    FE_CHECK(cmd);
    FE_CHECK(buffer);

    vkCmdDrawIndexedIndirect(cmd->vk().cmdBuffer, buffer->vk().buffer, offset, drawCount, stride);
}

void dispatch(CommandBuffer* cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    FE_CHECK(cmd);

    vkCmdDispatch(cmd->vk().cmdBuffer, groupCountX, groupCountY, groupCountZ);
}

void dispatch_mesh(CommandBuffer* cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    FE_CHECK(cmd);
    vkCmdDrawMeshTasksEXT(cmd->vk().cmdBuffer, groupCountX, groupCountY, groupCountZ);
}

void dispatch_rays(CommandBuffer* cmd, const DispatchRaysInfo* info)
{
    FE_CHECK(cmd);

    auto fillDeviceAddress = [&](
        const ShaderIdentifierBuffer& inIdentifier,
        VkStridedDeviceAddressRegionKHR& outRegion
    )
    {
        outRegion.deviceAddress = inIdentifier.buffer ? inIdentifier.buffer->vk().address : 0;
        outRegion.deviceAddress += inIdentifier.offset;
        outRegion.size = inIdentifier.size;
        outRegion.stride = inIdentifier.stride;
    };

    VkStridedDeviceAddressRegionKHR raygenDeviceAddress{};
    fillDeviceAddress(info->rayGeneration, raygenDeviceAddress);

    VkStridedDeviceAddressRegionKHR missDeviceAddress{};
    fillDeviceAddress(info->miss, missDeviceAddress);

    VkStridedDeviceAddressRegionKHR hitGroupDeviceAddress{};
    fillDeviceAddress(info->hitGroup, hitGroupDeviceAddress);

    VkStridedDeviceAddressRegionKHR callableDeviceAddress{};
    fillDeviceAddress(info->callable, callableDeviceAddress);

    vkCmdTraceRaysKHR(
        cmd->vk().cmdBuffer,
        &raygenDeviceAddress,
        &missDeviceAddress,
        &hitGroupDeviceAddress,
        &callableDeviceAddress,
        info->width,
        info->height,
        info->depth
    );
}

void add_pipeline_barriers(CommandBuffer* cmd, const std::vector<PipelineBarrier>& barriers)
{
    FE_CHECK(cmd);
    if (barriers.empty())
        return;

    std::vector<VkMemoryBarrier2> memoryBarriers;
    std::vector<VkBufferMemoryBarrier2> bufferBarriers;
    std::vector<VkImageMemoryBarrier2> imageBarriers;

    for (auto& pipelineBarrier : barriers)
    {
        switch (pipelineBarrier.type)
        {
        case rhi::PipelineBarrier::MEMORY:
        {
            const rhi::PipelineBarrier::MemoryBarrier* rhiMemoryBarrier = pipelineBarrier.get_memory_barrier();

            VkMemoryBarrier2 memoryBarrier{};
            memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
            memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            memoryBarrier.srcAccessMask = get_access(rhiMemoryBarrier->srcLayout);
            memoryBarrier.dstAccessMask = get_access(rhiMemoryBarrier->dstLayout);
            memoryBarriers.push_back(memoryBarrier);
            break;
        }
        case rhi::PipelineBarrier::BUFFER:
        {
            const rhi::PipelineBarrier::BufferBarrier* rhiBufferBarrier = pipelineBarrier.get_buffer_barrier();
            const Buffer* buffer = rhiBufferBarrier->buffer;

            VkBufferMemoryBarrier2 bufferBarrier{};
            bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            bufferBarrier.buffer = buffer->vk().buffer;
            bufferBarrier.offset = 0;
            
            if (has_flag(buffer->bufferUsage, rhi::ResourceUsage::STORAGE_BUFFER))
            {
                bufferBarrier.size = VK_WHOLE_SIZE;
            }
            else
            {
                bufferBarrier.size = buffer->size;
            }

            bufferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            bufferBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            bufferBarrier.srcAccessMask = get_access(rhiBufferBarrier->srcLayout);
            bufferBarrier.dstAccessMask = get_access(rhiBufferBarrier->dstLayout);
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarriers.push_back(bufferBarrier);
            break;
        }
        case rhi::PipelineBarrier::TEXTURE:
        {
            const rhi::PipelineBarrier::TextureBarrier* rhiTextureBarrier = pipelineBarrier.get_texture_barrier();
            const Texture* texture = rhiTextureBarrier->texture;

            VkImageMemoryBarrier2 imageBarrier{};
            imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageBarrier.srcAccessMask = get_access(rhiTextureBarrier->srcLayout);
            imageBarrier.dstAccessMask = get_access(rhiTextureBarrier->dstLayout);
            imageBarrier.oldLayout = get_image_layout(rhiTextureBarrier->srcLayout);
            imageBarrier.newLayout = get_image_layout(rhiTextureBarrier->dstLayout);
            imageBarrier.image = texture->vk().image;

            VkImageSubresourceRange range;
            if (has_flag(texture->textureUsage, rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT))
            {
                range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            else
            {
                range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }

            range.layerCount = !rhiTextureBarrier->layerCount ? VK_REMAINING_ARRAY_LAYERS : rhiTextureBarrier->layerCount;
            range.baseArrayLayer = rhiTextureBarrier->baseLayer;
            range.levelCount = !rhiTextureBarrier->levelCount ? VK_REMAINING_MIP_LEVELS : rhiTextureBarrier->levelCount;
            range.baseMipLevel = rhiTextureBarrier->baseMipLevel;
            imageBarrier.subresourceRange = range;
            imageBarriers.push_back(imageBarrier);
            break;
        }
        }
    }

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.memoryBarrierCount = memoryBarriers.size();
    dependencyInfo.pMemoryBarriers = memoryBarriers.data();
    dependencyInfo.bufferMemoryBarrierCount = bufferBarriers.size();
    dependencyInfo.pBufferMemoryBarriers = bufferBarriers.data();
    dependencyInfo.imageMemoryBarrierCount = imageBarriers.size();
    dependencyInfo.pImageMemoryBarriers = imageBarriers.data();

    vkCmdPipelineBarrier2(cmd->vk().cmdBuffer, &dependencyInfo);
}

void acquire_next_image(SwapChain* swapChain, Semaphore* signalSemaphore, Fence* fence, uint32* frameIndex)
{
    FE_CHECK(swapChain);
    FE_CHECK(signalSemaphore);
    FE_CHECK(fence);
    FE_CHECK(frameIndex);

    VkResult res = vkAcquireNextImageKHR(g_device.device, swapChain->vk().swapChain, 1000000000, signalSemaphore->vk().semaphore, fence->vk().fence, frameIndex);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
        create_swap_chain_internal(swapChain);

    swapChain->vk().imageIndex = *frameIndex;
}

void submit(SubmitInfo* submitInfo, rhi::Fence* signalFence)
{
    FE_CHECK(submitInfo);

    std::vector<VkCommandBufferSubmitInfo> cmdSubmitInfos;
    cmdSubmitInfos.reserve(submitInfo->cmdBuffers.size());

    std::vector<VkSemaphoreSubmitInfo> waitSemaphoreSubmitInfos;
    waitSemaphoreSubmitInfos.reserve(submitInfo->waitSemaphores.size());

    std::vector<VkSemaphoreSubmitInfo> signalSemaphoreSubmitInfos;
    signalSemaphoreSubmitInfos.reserve(submitInfo->signalSemaphores.size());
    
    VkPipelineStageFlags2 stageMask;
    
    switch (submitInfo->queueType)
    {
    case QueueType::GRAPHICS:
        stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        break;
    case QueueType::COMPUTE:
        stageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        break;
    case QueueType::TRANSFER:
        stageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        break;
    default:
        FE_LOG(LogVulkanRHI, FATAL, "Undefined queue type");
        break;
    }
    
    for (CommandBuffer* cmd : submitInfo->cmdBuffers)
    {
        FE_CHECK(cmd);
        VkCommandBufferSubmitInfo& cmdSubmitInfo = cmdSubmitInfos.emplace_back();
        cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdSubmitInfo.commandBuffer = cmd->vk().cmdBuffer;
        cmdSubmitInfo.deviceMask = 0;
    }
    
    for (Semaphore* semaphore : submitInfo->waitSemaphores)
    {
        FE_CHECK(semaphore);
        VkSemaphoreSubmitInfo& semaphoreSubmitInfo = waitSemaphoreSubmitInfos.emplace_back();
        semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        semaphoreSubmitInfo.semaphore = semaphore->vk().semaphore;
        semaphoreSubmitInfo.stageMask = stageMask;
        semaphoreSubmitInfo.deviceIndex = 0;
    }
    
    for (Semaphore* semaphore : submitInfo->signalSemaphores)
    {
        FE_CHECK(semaphore);
        VkSemaphoreSubmitInfo& semaphoreSubmitInfo = signalSemaphoreSubmitInfos.emplace_back();
        semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        semaphoreSubmitInfo.semaphore = semaphore->vk().semaphore;
        semaphoreSubmitInfo.stageMask = stageMask;
        semaphoreSubmitInfo.deviceIndex = 0;
    }
    
    VkSubmitInfo2 vkSubmitInfo{};
    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    vkSubmitInfo.commandBufferInfoCount = cmdSubmitInfos.size();
    vkSubmitInfo.pCommandBufferInfos = cmdSubmitInfos.data();
    vkSubmitInfo.waitSemaphoreInfoCount = waitSemaphoreSubmitInfos.size();
    vkSubmitInfo.pWaitSemaphoreInfos = waitSemaphoreSubmitInfos.data();
    vkSubmitInfo.signalSemaphoreInfoCount = signalSemaphoreSubmitInfos.size();
    vkSubmitInfo.pSignalSemaphoreInfos = signalSemaphoreSubmitInfos.data();

    VkFence fence = signalFence ? signalFence->vk().fence : VK_NULL_HANDLE;

    VK_CHECK(vkQueueSubmit2(
        g_device.get_queue(submitInfo->queueType).handle, 
        1, 
        &vkSubmitInfo, 
        fence
    ));
}

void present(PresentInfo* presentInfo)
{
    FE_CHECK(presentInfo);

    const std::vector<SwapChain*> swapChains = presentInfo->swapChains;

    std::vector<VkSwapchainKHR> swapChainHandles(swapChains.size());
    std::vector<uint32_t> imageIndices(swapChains.size());

    for (uint32_t i = 0; i != swapChains.size(); ++i)
    {
        swapChainHandles[i] = swapChains[i]->vk().swapChain;
        imageIndices[i] = swapChains[i]->vk().imageIndex;
    }

    std::vector<VkSemaphore> waitSemaphores;
    for (Semaphore* semaphore : presentInfo->waitSemaphores)
    {
        waitSemaphores.push_back(semaphore->vk().semaphore);
    }
    
    VkPresentInfoKHR vkPresentInfo{};
    vkPresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    vkPresentInfo.pSwapchains = swapChainHandles.data();
    vkPresentInfo.swapchainCount = swapChains.size();
    vkPresentInfo.pWaitSemaphores = waitSemaphores.data();
    vkPresentInfo.waitSemaphoreCount = waitSemaphores.size();
    vkPresentInfo.pImageIndices = imageIndices.data();
    VkResult result = vkQueuePresentKHR(g_device.get_queue(QueueType::GRAPHICS).handle, &vkPresentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        for (auto& swapChain : swapChains)
        {
            create_swap_chain_internal(swapChain);
        }
    }
}

void wait_queue_idle(QueueType queueType)
{
    VK_CHECK(vkQueueWaitIdle(g_device.get_queue(queueType).handle));
}

void wait_for_fences(const std::vector<Fence*>& fences)
{
    if (fences.empty())
        return;

    std::vector<VkFence> vkFences;
    vkFences.reserve(fences.size());

    for (Fence* fence : fences)
    {
        vkFences.push_back(fence->vk().fence);
    }

    VK_CHECK(vkWaitForFences(g_device.device, vkFences.size(), vkFences.data(), VK_TRUE, 1000000000));
    VK_CHECK(vkResetFences(g_device.device, vkFences.size(), vkFences.data()));
}

API get_api()
{
    return API::VK;
}

void set_frame_index(uint64 frameIndex)
{
    g_frameIndex = frameIndex;
}

void set_name(ResourceVariant resource, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.pObjectName = name.c_str();

    if (Buffer** ppBuffer = std::get_if<Buffer*>(&resource))
    {
        Buffer* buffer = *ppBuffer;
        if (buffer->vk().buffer == VK_NULL_HANDLE) return;

        info.objectType = VK_OBJECT_TYPE_BUFFER;
        info.objectHandle = (uint64)buffer->vk().buffer;
    }
    else if (BufferView** ppBufferView = std::get_if<BufferView*>(&resource))
    {
        BufferView* bufferView = *ppBufferView;
        if (bufferView->vk().bufferView == VK_NULL_HANDLE) return;

        info.objectType = VK_OBJECT_TYPE_BUFFER_VIEW;
        info.objectHandle = (uint64)bufferView->vk().bufferView;
    }
    else if (Texture** ppTexture = std::get_if<Texture*>(&resource))
    {
        Texture* texture = *ppTexture;
        if (texture->vk().image == VK_NULL_HANDLE) return;

        info.objectType = VK_OBJECT_TYPE_IMAGE;
        info.objectHandle = (uint64)texture->vk().image;
    }
    else if (TextureView** ppTextureView = std::get_if<TextureView*>(&resource))
    {
        TextureView* textureView = *ppTextureView;
        if (textureView->vk().imageView == VK_NULL_HANDLE) return;

        info.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
        info.objectHandle = (uint64)textureView->vk().imageView;
    }
    else if (Shader** ppShader = std::get_if<Shader*>(&resource))
    {
        Shader* shader = *ppShader;
        if (shader->vk().shader == VK_NULL_HANDLE) return;

        info.objectType = VK_OBJECT_TYPE_SHADER_MODULE;
        info.objectHandle = (uint64)shader->vk().shader;
    }
    else if (Sampler** ppSampler = std::get_if<Sampler*>(&resource))
    {
        Sampler* sampler = *ppSampler;
        if (sampler->vk().sampler == VK_NULL_HANDLE) return;

        info.objectType = VK_OBJECT_TYPE_SAMPLER;
        info.objectHandle = (uint64)sampler->vk().sampler;
    }
    else if (Pipeline** ppPipeline = std::get_if<Pipeline*>(&resource))
    {
        Pipeline* pipeline = *ppPipeline;
        if (pipeline->vk().pipeline == VK_NULL_HANDLE) return;

        info.objectType = VK_OBJECT_TYPE_PIPELINE;
        info.objectHandle = (uint64)pipeline->vk().pipeline;
    }
    else if (CommandPool** ppCmdPool = std::get_if<CommandPool*>(&resource))
    {
        CommandPool* cmdPool = *ppCmdPool;
        if (cmdPool->vk().cmdPool == VK_NULL_HANDLE) return;

        info.objectType = VK_OBJECT_TYPE_COMMAND_POOL;
        info.objectHandle = (uint64)cmdPool->vk().cmdPool;
    }
    else if (CommandBuffer** ppCmdBuffer = std::get_if<CommandBuffer*>(&resource))
    {
        CommandBuffer* cmdBuffer = *ppCmdBuffer;
        if (cmdBuffer->vk().cmdBuffer == VK_NULL_HANDLE) return;

        info.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
        info.objectHandle = (uint64)cmdBuffer->vk().cmdBuffer;
    }
    else if (SwapChain** ppSwapChain = std::get_if<SwapChain*>(&resource))
    {
        SwapChain* swapChain = *ppSwapChain;
        if (swapChain->vk().swapChain == VK_NULL_HANDLE) return;

        info.objectType = VK_OBJECT_TYPE_SWAPCHAIN_KHR;
        info.objectHandle = (uint64)swapChain->vk().swapChain;
    }
    else if (Fence** ppFence = std::get_if<Fence*>(&resource))
    {
        Fence* fence = *ppFence;
        if (fence->vk().fence == VK_NULL_HANDLE) return;

        info.objectType = VK_OBJECT_TYPE_FENCE;
        info.objectHandle = (uint64)fence->vk().fence;
    }
    else if (Semaphore** ppSemaphore = std::get_if<Semaphore*>(&resource))
    {
        Semaphore* semaphore = *ppSemaphore;
        if (semaphore->vk().semaphore == VK_NULL_HANDLE) return;

        info.objectType = VK_OBJECT_TYPE_SEMAPHORE;
        info.objectHandle = (uint64)semaphore->vk().semaphore;
    }
    else if (AccelerationStructure** ppAS = std::get_if<AccelerationStructure*>(&resource))
    {
        AccelerationStructure* as = *ppAS;
        if (as->vk().accelerationStructure == VK_NULL_HANDLE) return;

        info.objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
        info.objectHandle = (uint64)as->vk().accelerationStructure;
    }

    VK_CHECK(vkSetDebugUtilsObjectNameEXT(g_device.device, &info));
}

uint64 get_min_offset_alignment(const BufferInfo* bufferInfo)
{
    uint64 alignment = 0;

    if (has_flag(bufferInfo->bufferUsage, rhi::ResourceUsage::STORAGE_BUFFER))
        alignment = std::max(alignment, g_device.properties2.properties.limits.minStorageBufferOffsetAlignment);

    if (has_flag(bufferInfo->bufferUsage, rhi::ResourceUsage::STORAGE_TEXEL_BUFFER))
        alignment = std::max(alignment, g_device.properties2.properties.limits.minTexelBufferOffsetAlignment);

    return alignment;
}

const GPUProperties& get_gpu_properties()
{
    return g_device.gpuProperties;
}

#pragma endregion

void fill_function_table()
{
    FE_LOG(LogVulkanRHI, INFO, "Starting Vulkan func table initialization.");

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
    fe::rhi::create_graphics_pipeline = create_graphics_pipeline;
    fe::rhi::create_compute_pipelines = create_compute_pipelines;
    fe::rhi::create_compute_pipeline = create_compute_pipeline;
    fe::rhi::create_ray_tracing_pipeline = create_ray_tracing_pipeline;
    fe::rhi::destroy_pipeline = destroy_pipeline;

    fe::rhi::create_acceleration_structure = create_acceleration_structure;
    fe::rhi::destroy_acceleration_structure = destroy_acceleration_structure;
    fe::rhi::write_top_level_acceleration_structure_instance = write_top_level_acceleration_structure_instance;
    fe::rhi::write_shader_identifier = write_shader_identifier;

    fe::rhi::bind_uniform_buffer = bind_uniform_buffer;

    fe::rhi::create_command_pool = create_command_pool;
    fe::rhi::destroy_command_pool = destroy_command_pool;
    fe::rhi::create_command_buffer = create_command_buffer;
    fe::rhi::destroy_command_buffer = destroy_command_buffer;
    fe::rhi::begin_command_buffer = begin_command_buffer;
    fe::rhi::end_command_buffer = end_command_buffer;
    fe::rhi::reset_command_pool = reset_command_pool;

    fe::rhi::create_semaphore = create_semaphore;
    fe::rhi::destroy_semaphore = destroy_semaphore;
    fe::rhi::create_fence = create_fence;
    fe::rhi::destroy_fence = destroy_fence;

    fe::rhi::fill_buffer = fill_buffer;
    fe::rhi::copy_buffer = copy_buffer;
    fe::rhi::copy_texture = copy_texture;
    fe::rhi::init_texture = init_texture;
    fe::rhi::copy_buffer_to_texture = copy_buffer_to_texture;
    fe::rhi::copy_texture_to_buffer = copy_texture_to_buffer;
    fe::rhi::blit_texture = blit_texture;

    fe::rhi::set_viewports = set_viewports;
    fe::rhi::set_scissors = set_scissors;
    
    fe::rhi::push_constants = push_constants;
    fe::rhi::bind_vertex_buffer = bind_vertex_buffer;
    fe::rhi::bind_index_buffer = bind_index_buffer;
    fe::rhi::bind_pipeline = bind_pipeline;

    fe::rhi::build_acceleration_structure = build_acceleration_structure;

    fe::rhi::begin_rendering = begin_rendering;
    fe::rhi::end_rendering = end_rendering;

    fe::rhi::draw = draw;
    fe::rhi::draw_indexed = draw_indexed;
    fe::rhi::draw_indirect = draw_indirect;
    fe::rhi::draw_indexed_indirect = draw_indexed_indirect;

    fe::rhi::dispatch = dispatch;
    fe::rhi::dispatch_mesh = dispatch_mesh;
    fe::rhi::dispatch_rays = dispatch_rays;
    fe::rhi::add_pipeline_barriers = add_pipeline_barriers;

    fe::rhi::acquire_next_image = acquire_next_image;
    fe::rhi::submit = submit;
    fe::rhi::present = present;
    fe::rhi::wait_queue_idle = wait_queue_idle;
    fe::rhi::wait_for_fences = wait_for_fences;

    fe::rhi::get_api = get_api;
    fe::rhi::set_frame_index = set_frame_index;
    fe::rhi::set_name = set_name;

    fe::rhi::get_min_offset_alignment = get_min_offset_alignment;

    fe::rhi::get_gpu_properties = get_gpu_properties;

    FE_LOG(LogVulkanRHI, INFO, "Vulkan func table initialization completed.");
}

}