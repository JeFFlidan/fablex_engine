#pragma once

#include "resources.h"
#include "core/logger.h"
#include "core/utils.h"

#ifdef FE_VULKAN
#include "rhi/vulkan/vulkan_rhi.h"
#endif // FE_VULKAN

namespace fe::rhi
{

inline void fill_function_table(API api)
{
    switch (api)
    {
    case API::VK:
#ifdef FE_VULKAN
        vulkan::fill_function_table();
#endif // FE_VULKAN
        break;
    case API::D3D12:
        break;
    }
}

inline bool support_stencil(Format format)
{
    if (format == Format::D32_SFLOAT || format == Format::D16_UNORM)
        return false;
    return true;
}

inline bool is_depth_stencil_format(Format format)
{
    switch (format)
    {
    case Format::D16_UNORM:
    case Format::D16_UNORM_S8_UINT:
    case Format::D24_UNORM_S8_UINT:
    case Format::D32_SFLOAT:
    case Format::D32_SFLOAT_S8_UINT:
        return true;
    default:
        return false;
    }
}

inline uint32_t get_format_stride(Format format)
{
    switch (format)
    {
        case Format::UNDEFINED:
            FE_LOG(LogDefault, FATAL, "rhi::Utils::get_format_stride(): Undefined format");
        case Format::R4G4_UNORM:
        case Format::R8_UNORM:
        case Format::R8_SNORM:
        case Format::R8_UINT:
        case Format::R8_SINT:
        case Format::R8_SRGB:
        case Format::S8_UINT:
            return 1u;
        case Format::R4G4B4A4_UNORM:
        case Format::B4G4R4A4_UNORM:
        case Format::R5G5B5A1_UNORM:
        case Format::B5G5R5A1_UNORM:
        case Format::A1R5G5B5_UNORM:
        case Format::R8G8_UNORM:
        case Format::R8G8_SNORM:
        case Format::R8G8_UINT:
        case Format::R8G8_SINT:
        case Format::R8G8_SRGB:
        case Format::R16_UNORM:
        case Format::R16_SNORM:
        case Format::R16_UINT:
        case Format::R16_SINT:
        case Format::R16_SFLOAT:
        case Format::D16_UNORM:
            return 2u;
        case Format::D16_UNORM_S8_UINT:
            return 3u;
        case Format::R8G8B8A8_UNORM:
        case Format::R8G8B8A8_SNORM:
        case Format::R8G8B8A8_UINT:
        case Format::R8G8B8A8_SINT:
        case Format::R8G8B8A8_SRGB:
        case Format::B8G8R8A8_SRGB:
        case Format::B8G8R8A8_UNORM:
        case Format::B8G8R8A8_SNORM:
        case Format::R10G10B10A2_UNORM:
        case Format::R10G10B10A2_SNORM:
        case Format::D24_UNORM_S8_UINT:
        case Format::D32_SFLOAT:
        case Format::R32_UINT:
        case Format::R32_SINT:
        case Format::R32_SFLOAT:
        case Format::R16G16_UNORM:
        case Format::R16G16_SNORM:
        case Format::R16G16_UINT:
        case Format::R16G16_SINT:
        case Format::R16G16_SFLOAT:
            return 4u;
        case Format::R16G16B16A16_UNORM:
        case Format::R16G16B16A16_SNORM:
        case Format::R16G16B16A16_UINT:
        case Format::R16G16B16A16_SINT:
        case Format::R16G16B16A16_SFLOAT:
        case Format::R32G32_UINT:
        case Format::R32G32_SINT:
        case Format::R32G32_SFLOAT:
        case Format::D32_SFLOAT_S8_UINT:
        case Format::BC1_RGBA_UNORM:
        case Format::BC1_RGBA_SRGB_UNORM:
        case Format::BC4_UNORM:
        case Format::BC4_SNORM:
            return 8u;
        case Format::R32G32B32_UINT:
        case Format::R32G32B32_SINT:
        case Format::R32G32B32_SFLOAT:
            return 12u;
        case Format::R32G32B32A32_UINT:
        case Format::R32G32B32A32_SINT:
        case Format::R32G32B32A32_SFLOAT:
        case Format::BC2_UNORM:
        case Format::BC2_SRGB:
        case Format::BC3_UNORM:
        case Format::BC3_SRGB:
        case Format::BC5_UNORM:
        case Format::BC5_SNORM:
        case Format::BC6H_UFLOAT:
        case Format::BC6H_SFLOAT:
        case Format::BC7_UNORM:
        case Format::BC7_SRGB:
            return 16u;
        default:
            FE_LOG(LogDefault, FATAL, "rhi::Utils::get_format_stride(): Format does not have stride");
            return 0u;
    }
}

inline Format get_non_srgb_format(Format format)
{
    switch (format)
    {
        case Format::R8_SRGB:
            return Format::R8_UNORM;
        case Format::R8G8_SRGB:
            return Format::R8G8_UNORM;
        case Format::R8G8B8A8_SRGB:
            return Format::R8G8B8A8_UNORM;
        case Format::B8G8R8A8_SRGB:
            return Format::B8G8R8A8_UNORM;
        default:
            return format;
    }
}

inline bool is_format_srgb(Format format)
{
    switch (format)
    {
        case Format::R8_SRGB:
        case Format::R8G8_SRGB:
        case Format::R8G8B8A8_SRGB:
        case Format::B8G8R8A8_SRGB:
            return true;
        default:
            return false;
    }
}

inline std::string format_to_str(Format format)
{
    switch (format)
    {
        case Format::R4G4_UNORM:
            return "R4G4_UNORM";
        case Format::R4G4B4A4_UNORM:
            return "R4G4B4A4_UNORM";
        case Format::B4G4R4A4_UNORM:
            return "B4G4R4A4_UNORM";
        case Format::R5G5B5A1_UNORM:
            return "R5G5B5A1_UNORM";
        case Format::B5G5R5A1_UNORM:
            return "B5G5R5A1_UNORM";
        case Format::A1R5G5B5_UNORM:
            return "A1R5G5B5_UNORM";

        case Format::R8_UNORM:
            return "R8_UNORM";
        case Format::R8_SNORM:
            return "R8_SNORM";
        case Format::R8_UINT:
            return "R8_UINT";
        case Format::R8_SINT:
            return "R8_SINT";
        case Format::R8_SRGB:
            return "R8_SRGB";

        case Format::R8G8_UNORM:
            return "R8G8_UNORM";
        case Format::R8G8_SNORM:
            return "R8G8_SNORM";
        case Format::R8G8_UINT:
            return "R8G8_UINT";
        case Format::R8G8_SINT:
            return "R8G8_SINT";
        case Format::R8G8_SRGB:
            return "R8G8_SRGB";

        case Format::R8G8B8A8_UNORM:
            return "R8G8B8A8_UNORM";
        case Format::R8G8B8A8_SNORM:
            return "R8G8B8A8_SNORM";
        case Format::R8G8B8A8_UINT:
            return "R8G8B8A8_UINT";
        case Format::R8G8B8A8_SINT:
            return "R8G8B8A8_SINT";
        case Format::R8G8B8A8_SRGB:
            return "R8G8B8A8_SRGB";

        case Format::B8G8R8A8_SRGB:
            return "B8G8R8A8_SRGB";
        case Format::B8G8R8A8_UNORM:
            return "B8G8R8A8_UNORM";
        case Format::B8G8R8A8_SNORM:
            return "B8G8R8A8_SNORM";

        case Format::R10G10B10A2_UNORM:
            return "R10G10B10A2_UNORM";
        case Format::R10G10B10A2_SNORM:
            return "R10G10B10A2_SNORM";

        case Format::R16_UNORM:
            return "R16_UNORM";
        case Format::R16_SNORM:
            return "R16_SNORM";
        case Format::R16_UINT:
            return "R16_UINT";
        case Format::R16_SINT:
            return "R16_SINT";
        case Format::R16_SFLOAT:
            return "R16_SFLOAT";

        case Format::R16G16_UNORM:
            return "R16G16_UNORM";
        case Format::R16G16_SNORM:
            return "R16G16_SNORM";
        case Format::R16G16_UINT:
            return "R16G16_UINT";
        case Format::R16G16_SINT:
            return "R16G16_SINT";
        case Format::R16G16_SFLOAT:
            return "R16G16_SFLOAT";

        case Format::R16G16B16A16_UNORM:
            return "R16G16B16A16_UNORM";
        case Format::R16G16B16A16_SNORM:
            return "R16G16B16A16_SNORM";
        case Format::R16G16B16A16_UINT:
            return "R16G16B16A16_UINT";
        case Format::R16G16B16A16_SINT:
            return "R16G16B16A16_SINT";
        case Format::R16G16B16A16_SFLOAT:
            return "R16G16B16A16_SFLOAT";

        case Format::R32_UINT:
            return "R32_UINT";
        case Format::R32_SINT:
            return "R32_SINT";
        case Format::R32_SFLOAT:
            return "R32_SFLOAT";
        case Format::R32G32_UINT:
            return "R32G32_UINT";
        case Format::R32G32_SINT:
            return "R32G32_SINT";
        case Format::R32G32_SFLOAT:
            return "R32G32_SFLOAT";

        case Format::R32G32B32_UINT:
            return "R32G32B32_UINT";
        case Format::R32G32B32_SINT:
            return "R32G32B32_SINT";
        case Format::R32G32B32_SFLOAT:
            return "R32G32B32_SFLOAT";

        case Format::R32G32B32A32_UINT:
            return "R32G32B32A32_UINT";
        case Format::R32G32B32A32_SINT:
            return "R32G32B32A32_SINT";
        case Format::R32G32B32A32_SFLOAT:
            return "R32G32B32A32_SFLOAT";

        case Format::D16_UNORM:
            return "D16_UNORM";
        case Format::D32_SFLOAT:
            return "D32_SFLOAT";

        case Format::S8_UINT:
            return "S8_UINT";
        case Format::D16_UNORM_S8_UINT:
            return "D16_UNORM_S8_UINT";
        case Format::D24_UNORM_S8_UINT:
            return "D24_UNORM_S8_UINT";
        case Format::D32_SFLOAT_S8_UINT:
            return "D32_SFLOAT_S8_UINT";
        
        case Format::BC1_RGBA_UNORM:
            return "BC1_RGBA_UNORM";
        case Format::BC1_RGBA_SRGB_UNORM:
            return "BC1_RGBA_SRGB_UNORM";
        case Format::BC2_UNORM:
            return "BC2_UNORM";
        case Format::BC2_SRGB:
            return "BC2_SRGB";
        case Format::BC3_UNORM:
            return "BC3_UNORM";
        case Format::BC3_SRGB:
            return "BC3_SRGB";
        case Format::BC4_UNORM:
            return "BC4_UNORM";
        case Format::BC4_SNORM:
            return "BC4_SNORM";
        case Format::BC5_UNORM:
            return "BC5_UNORM";
        case Format::BC5_SNORM:
            return "BC5_SNORM";
        case Format::BC6H_UFLOAT:
            return "BC6H_UFLOAT";
        case Format::BC6H_SFLOAT:
            return "BC6H_SFLOAT";
        case Format::BC7_UNORM:
            return "BC7_UNORM";
        case Format::BC7_SRGB:
            return "BC7_SRGB";
        default:
            FE_LOG(LogDefault, FATAL, "rhi::Utils::get_format_str(): Format is undefined");
            return "UNDEFINED";
    }
}

inline Format format_to_enum(const std::string& format)
{
    if (format == "R4G4_UNORM")
        return Format::R4G4_UNORM;
    if (format == "R4G4B4A4_UNORM")
        return Format::R4G4B4A4_UNORM;
    if (format == "B4G4R4A4_UNORM")
        return Format::B4G4R4A4_UNORM;
    if (format == "R5G5B5A1_UNORM")
        return Format::R5G5B5A1_UNORM;
    if (format == "B5G5R5A1_UNORM")
        return Format::B5G5R5A1_UNORM;
    if (format == "A1R5G5B5_UNORM")
        return Format::A1R5G5B5_UNORM;

    if (format == "R8_UNORM")
        return Format::R8_UNORM;
    if (format == "R8_SNORM")
        return Format::R8_SNORM;
    if (format == "R8_UINT")
        return Format::R8_UINT;
    if (format == "R8_SINT")
        return Format::R8_SINT;
    if (format == "R8_SRGB")
        return Format::R8_SRGB;

    if (format == "R8G8_UNORM")
        return Format::R8G8_UNORM;
    if (format == "R8G8_SNORM")
        return Format::R8G8_SNORM;
    if (format == "R8G8_UINT")
        return Format::R8G8_UINT;
    if (format == "R8G8_SINT")
        return Format::R8G8_SINT;
    if (format == "R8G8_SRGB")
        return Format::R8G8_SRGB;

    if (format == "R8G8B8A8_UNORM")
        return Format::R8G8B8A8_UNORM;
    if (format == "R8G8B8A8_SNORM")
        return Format::R8G8B8A8_SNORM;
    if (format == "R8G8B8A8_UINT")
        return Format::R8G8B8A8_UINT;
    if (format == "R8G8B8A8_SINT")
        return Format::R8G8B8A8_SINT;
    if (format == "R8G8B8A8_SRGB")
        return Format::R8G8B8A8_SRGB;

    if (format == "B8G8R8A8_SRGB")
        return Format::B8G8R8A8_SRGB;
    if (format == "B8G8R8A8_UNORM")
        return Format::B8G8R8A8_UNORM;
    if (format == "B8G8R8A8_SNORM")
        return Format::B8G8R8A8_SNORM;

    if (format == "R10G10B10A2_UNORM")
        return Format::R10G10B10A2_UNORM;
    if (format == "R10G10B10A2_SNORM")
        return Format::R10G10B10A2_SNORM;

    if (format == "R16_UNORM")
        return Format::R16_UNORM;
    if (format == "R16_SNORM")
        return Format::R16_SNORM;
    if (format == "R16_UINT")
        return Format::R16_UINT;
    if (format == "R16_SINT")
        return Format::R16_SINT;
    if (format == "R16_SFLOAT")
        return Format::R16_SFLOAT;

    if (format == "R16G16_UNORM")
        return Format::R16G16_UNORM;
    if (format == "R16G16_SNORM")
        return Format::R16G16_SNORM;
    if (format == "R16G16_UINT")
        return Format::R16G16_UINT;
    if (format == "R16G16_SINT")
        return Format::R16G16_SINT;
    if (format == "R16G16_SFLOAT")
        return Format::R16G16_SFLOAT;

    if (format == "R16G16B16A16_UNORM")
        return Format::R16G16B16A16_UNORM;
    if (format == "R16G16B16A16_SNORM")
        return Format::R16G16B16A16_SNORM;
    if (format == "R16G16B16A16_UINT")
        return Format::R16G16B16A16_UINT;
    if (format == "R16G16B16A16_SINT")
        return Format::R16G16B16A16_SINT;
    if (format == "R16G16B16A16_SFLOAT")
        return Format::R16G16B16A16_SFLOAT;

    if (format == "R32_UINT")
        return Format::R32_UINT;
    if (format == "R32_SINT")
        return Format::R32_SINT;
    if (format == "R32_SFLOAT")
        return Format::R32_SFLOAT;
    if (format == "R32G32_UINT")
        return Format::R32G32_UINT;
    if (format == "R32G32_SINT")
        return Format::R32G32_SINT;
    if (format == "R32G32_SFLOAT")
        return Format::R32G32_SFLOAT;

    if (format == "R32G32B32_UINT")
        return Format::R32G32B32_UINT;
    if (format == "R32G32B32_SINT")
        return Format::R32G32B32_SINT;
    if (format == "R32G32B32_SFLOAT")
        return Format::R32G32B32_SFLOAT;

    if (format == "R32G32B32A32_UINT")
        return Format::R32G32B32A32_UINT;
    if (format == "R32G32B32A32_SINT")
        return Format::R32G32B32A32_SINT;
    if (format == "R32G32B32A32_SFLOAT")
        return Format::R32G32B32A32_SFLOAT;

    if (format == "D16_UNORM")
        return Format::D16_UNORM;
    if (format == "D32_SFLOAT")
        return Format::D32_SFLOAT;

    if (format == "S8_UINT")
        return Format::S8_UINT;
    if (format == "D16_UNORM_S8_UINT")
        return Format::D16_UNORM_S8_UINT;
    if (format == "D24_UNORM_S8_UINT")
        return Format::D24_UNORM_S8_UINT;
    if (format == "D32_SFLOAT_S8_UINT")
        return Format::D32_SFLOAT_S8_UINT;
    
    if (format == "BC1_RGBA_UNORM")
        return Format::BC1_RGBA_UNORM;
    if (format == "BC1_RGBA_SRGB_UNORM")
        return Format::BC1_RGBA_SRGB_UNORM;
    if (format == "BC2_UNORM")
        return Format::BC2_UNORM;
    if (format == "BC2_SRGB")
        return Format::BC2_SRGB;
    if (format == "BC3_UNORM")
        return Format::BC3_UNORM;
    if (format == "BC3_SRGB")
        return Format::BC3_SRGB;
    if (format == "BC4_UNORM")
        return Format::BC4_UNORM;
    if (format == "BC4_SNORM")
        return Format::BC4_SNORM;
    if (format == "BC5_UNORM")
        return Format::BC5_UNORM;
    if (format == "BC5_SNORM")
        return Format::BC5_SNORM;
    if (format == "BC6H_UFLOAT")
        return Format::BC6H_UFLOAT;
    if (format == "BC6H_SFLOAT")
        return Format::BC6H_SFLOAT;
    if (format == "BC7_UNORM")
        return Format::BC7_UNORM;
    if (format == "BC7_SRGB")
        return Format::BC7_SRGB;
    FE_LOG(LogDefault, FATAL, "rhi::Utils::get_format_enum(): Format is undefined");
    return Format::UNDEFINED;
}

inline char component_swizzle_to_str(ComponentSwizzle swizzle)
{
    switch (swizzle)
    {
        case ComponentSwizzle::R:
            return 'R';
        case ComponentSwizzle::G:
            return 'G';
        case ComponentSwizzle::B:
            return 'B';
        case ComponentSwizzle::A:
            return 'A';
        case ComponentSwizzle::ONE:
            return '1';
        case ComponentSwizzle::ZERO:
            return '0';
        default:
            FE_LOG(LogDefault, FATAL, "rhi::Utils::get_comonent_swizzle_str(): Swizzle is undefined");
            return 'U';
    }
}

inline ComponentSwizzle component_swizzle_to_enum(char swizzle)
{
    switch (swizzle)
    {
        case 'R':
            return ComponentSwizzle::R;
        case 'G':
            return ComponentSwizzle::G;
        case 'B':
            return ComponentSwizzle::B;
        case 'A':
            return ComponentSwizzle::A;
        case '1':
            return ComponentSwizzle::ONE;
        case '0':
            return ComponentSwizzle::ZERO;
        default:
            FE_LOG(LogDefault, FATAL, "rhi::Utils::get_component_swizzle_enum(): Swizzle is undefined");
            return ComponentSwizzle::UNDEFINED;
    }
}

inline std::string component_mapping_to_str(ComponentMapping mapping)
{
    std::string result;
    result.resize(4);
    result[0] = component_swizzle_to_str(mapping.r);
    result[1] = component_swizzle_to_str(mapping.g);
    result[2] = component_swizzle_to_str(mapping.b);
    result[3] = component_swizzle_to_str(mapping.a);
    return result;
}

inline ComponentMapping component_mapping_to_enum(const std::string& mappingStr)
{
    ComponentMapping componentMapping;
    if (mappingStr.size() != 4)
    {
        FE_LOG(LogDefault, FATAL, "rhi::Utils::get_component_mapping(): Mapping string is invalid");
        return componentMapping;
    }
    componentMapping.r = component_swizzle_to_enum(mappingStr[0]);
    componentMapping.g = component_swizzle_to_enum(mappingStr[1]);
    componentMapping.b = component_swizzle_to_enum(mappingStr[2]);
    componentMapping.a = component_swizzle_to_enum(mappingStr[3]);
    return componentMapping;
}

inline bool is_component_mapping_valid(const ComponentMapping& mapping)
{
    return mapping.r != ComponentSwizzle::UNDEFINED &&
        mapping.g != ComponentSwizzle::UNDEFINED &&
        mapping.b != ComponentSwizzle::UNDEFINED &&
        mapping.a != ComponentSwizzle::UNDEFINED;
}

inline bool is_format_block_compressed(Format format)
{
    switch (format)
    {
        case Format::BC1_RGBA_UNORM:
        case Format::BC1_RGBA_SRGB_UNORM:
        case Format::BC2_UNORM:
        case Format::BC2_SRGB:
        case Format::BC3_UNORM:
        case Format::BC3_SRGB:
        case Format::BC4_UNORM:
        case Format::BC4_SNORM:
        case Format::BC5_UNORM:
        case Format::BC5_SNORM:
        case Format::BC6H_UFLOAT:
        case Format::BC6H_SFLOAT:
        case Format::BC7_UNORM:
        case Format::BC7_SRGB:
            return true;
        default:
            return false;
    }
}

inline uint32 get_block_format_size(Format format)
{
    if (is_format_block_compressed(format))
        return 4;
    return 1;
}

inline bool is_rt_shader(rhi::ShaderType shaderType)
{
    switch (shaderType)
    {
    case rhi::ShaderType::LIB:
    case rhi::ShaderType::RAY_GENERATION:
    case rhi::ShaderType::RAY_MISS:
    case rhi::ShaderType::RAY_CLOSEST_HIT:
    case rhi::ShaderType::RAY_ANY_HIT:
    case rhi::ShaderType::RAY_CALLABLE:
    case rhi::ShaderType::RAY_INTERSECTION:
        return true;
    default:
        return false;
    }
}

inline uint32 get_queue_index(rhi::QueueType queueType)
{
    return std::underlying_type_t<rhi::QueueType>(queueType);
}

// From wicked engine
template<typename T>
T align_to(T value, T alignment)
{
    return ((value + alignment - T(1)) / alignment) * alignment;
}

// From wicked engine
template<typename T>
bool is_aligned(T value, T alignment)
{
    return value == align_to(value, alignment);
}

}