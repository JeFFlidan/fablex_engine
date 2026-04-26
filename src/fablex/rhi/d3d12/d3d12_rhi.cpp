#include "core/platform/platform.h"

#include "d3d12.h"
#include "rhi/rhi.h"
#include "rhi/macro.h"
#include "rhi/utils.h"
#include "rhi/resources.h"
#include "core/logger.h"

FE_DEFINE_LOG_CATEGORY(LogD3D12);

namespace fe::rhi::d3d12
{

//============================================================================================================================================================================================
#pragma region [ UTILITY ]
//============================================================================================================================================================================================

DXGI_FORMAT get_format(Format format)
{
    switch (format)
    {
        case Format::UNDEFINED:
            return DXGI_FORMAT_UNKNOWN;
        case Format::R4G4_UNORM:
        case Format::R4G4B4A4_UNORM:
        case Format::R5G5B5A1_UNORM:
        case Format::A1R5G5B5_UNORM:
            break;
        case Format::B4G4R4A4_UNORM:
            return DXGI_FORMAT_B4G4R4A4_UNORM;
        case Format::B5G5R5A1_UNORM:
            return DXGI_FORMAT_B5G5R5A1_UNORM;

        case Format::R8_UNORM:
            return DXGI_FORMAT_R8_UNORM;
        case Format::R8_SNORM:
            return DXGI_FORMAT_R8_SNORM;
        case Format::R8_UINT:
            return DXGI_FORMAT_R8_UINT;
        case Format::R8_SINT:
            return DXGI_FORMAT_R8_SINT;
        case Format::R8_SRGB:
            break;

        case Format::R8G8_UNORM:
            return DXGI_FORMAT_R8G8_UNORM;
        case Format::R8G8_SNORM:
            return DXGI_FORMAT_R8G8_SNORM;
        case Format::R8G8_UINT:
            return DXGI_FORMAT_R8G8_UINT;
        case Format::R8G8_SINT:
            return DXGI_FORMAT_R8G8_SINT;
        case Format::R8G8_SRGB:
            break;

        case Format::R8G8B8A8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case Format::R8G8B8A8_SNORM:
            return DXGI_FORMAT_R8G8B8A8_SNORM;
        case Format::R8G8B8A8_UINT:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case Format::R8G8B8A8_SINT:
            return DXGI_FORMAT_R8G8B8A8_SINT;
        case Format::R8G8B8A8_SRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

        case Format::B8G8R8A8_SRGB:
            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        case Format::B8G8R8A8_UNORM:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case Format::B8G8R8A8_SNORM:
            break;

        case Format::R10G10B10A2_UNORM:
            return DXGI_FORMAT_R10G10B10A2_UNORM;
        case Format::R10G10B10A2_SNORM:
            break;

        case Format::R16_UNORM:
            return DXGI_FORMAT_R16_UNORM;
        case Format::R16_SNORM:
            return DXGI_FORMAT_R16_SNORM;
        case Format::R16_UINT:
            return DXGI_FORMAT_R16_UINT;
        case Format::R16_SINT:
            return DXGI_FORMAT_R16_SINT;
        case Format::R16_SFLOAT:
            return DXGI_FORMAT_R16_FLOAT;

        case Format::R16G16_UNORM:
            return DXGI_FORMAT_R16G16_UNORM;
        case Format::R16G16_SNORM:
            return DXGI_FORMAT_R16G16_SNORM;
        case Format::R16G16_UINT:
            return DXGI_FORMAT_R16G16_UINT;
        case Format::R16G16_SINT:
            return DXGI_FORMAT_R16G16_SINT;
        case Format::R16G16_SFLOAT:
            return DXGI_FORMAT_R16G16_FLOAT;

        case Format::R16G16B16A16_UNORM:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case Format::R16G16B16A16_SNORM:
            return DXGI_FORMAT_R16G16B16A16_SNORM;
        case Format::R16G16B16A16_UINT:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case Format::R16G16B16A16_SINT:
            return DXGI_FORMAT_R16G16B16A16_SINT;
        case Format::R16G16B16A16_SFLOAT:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;

        case Format::R32_UINT:
            return DXGI_FORMAT_R32_UINT;
        case Format::R32_SINT:
            return DXGI_FORMAT_R32_SINT;
        case Format::R32_SFLOAT:
            return DXGI_FORMAT_R32_FLOAT;
        case Format::R32G32_UINT:
            return DXGI_FORMAT_R32G32_UINT;
        case Format::R32G32_SINT:
            return DXGI_FORMAT_R32G32_SINT;
        case Format::R32G32_SFLOAT:
            return DXGI_FORMAT_R32G32_FLOAT;

        case Format::R32G32B32_UINT:
            return DXGI_FORMAT_R32G32B32_UINT;
        case Format::R32G32B32_SINT:
            return DXGI_FORMAT_R32G32B32_SINT;
        case Format::R32G32B32_SFLOAT:
            return DXGI_FORMAT_R32G32B32_FLOAT;

        case Format::R32G32B32A32_UINT:
            return DXGI_FORMAT_R32G32B32A32_UINT;
        case Format::R32G32B32A32_SINT:
            return DXGI_FORMAT_R32G32B32A32_SINT;
        case Format::R32G32B32A32_SFLOAT:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;

        case Format::D16_UNORM:
            return DXGI_FORMAT_D16_UNORM;
        case Format::D32_SFLOAT:
            return DXGI_FORMAT_D32_FLOAT;

        case Format::S8_UINT:
            break;
        case Format::D16_UNORM_S8_UINT:
            break;
        case Format::D24_UNORM_S8_UINT:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case Format::D32_SFLOAT_S8_UINT:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            
        case Format::BC1_RGBA_UNORM:
            return DXGI_FORMAT_BC1_UNORM;
        case Format::BC1_RGBA_SRGB_UNORM:
            return DXGI_FORMAT_BC1_UNORM_SRGB;
        case Format::BC2_UNORM:
            return DXGI_FORMAT_BC2_UNORM;
        case Format::BC2_SRGB:
            return DXGI_FORMAT_BC2_UNORM_SRGB;
        case Format::BC3_UNORM:
            return DXGI_FORMAT_BC3_UNORM;
        case Format::BC3_SRGB:
            return DXGI_FORMAT_BC3_UNORM_SRGB;
        case Format::BC4_UNORM:
            return DXGI_FORMAT_BC4_UNORM;
        case Format::BC4_SNORM:
            return DXGI_FORMAT_BC4_SNORM;
        case Format::BC5_UNORM:
            return DXGI_FORMAT_BC5_UNORM;
        case Format::BC5_SNORM:
            return DXGI_FORMAT_BC5_SNORM;
        case Format::BC6H_UFLOAT:
            return DXGI_FORMAT_BC6H_UF16;
        case Format::BC6H_SFLOAT:
            return DXGI_FORMAT_BC6H_SF16;
        case Format::BC7_UNORM:
            return DXGI_FORMAT_BC7_UNORM;
        case Format::BC7_SRGB:
            return DXGI_FORMAT_BC7_UNORM_SRGB;
    }

    FE_LOG(LogD3D12, ERROR, "D3D12 does not support format {}!", format_to_str(format));
    return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT format_to_typeless(DXGI_FORMAT format)
{
    switch (format)
    {
        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SINT:
            return DXGI_FORMAT_R8_TYPELESS;

        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SINT:
            return DXGI_FORMAT_R8G8_TYPELESS;

        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return DXGI_FORMAT_R8G8B8A8_TYPELESS;

        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return DXGI_FORMAT_B8G8R8A8_TYPELESS;

        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
            return DXGI_FORMAT_R10G10B10A2_TYPELESS;

        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_R16_FLOAT:
            return DXGI_FORMAT_R16_TYPELESS;

        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R16G16_FLOAT:
            return DXGI_FORMAT_R16G16_TYPELESS;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            return DXGI_FORMAT_R16G16B16A16_TYPELESS;

        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R32_FLOAT:
            return DXGI_FORMAT_R32_TYPELESS;

        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G32_FLOAT:
            return DXGI_FORMAT_R32G32_TYPELESS;

        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
        case DXGI_FORMAT_R32G32B32_FLOAT:
            return DXGI_FORMAT_R32G32B32_TYPELESS;

        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
            return DXGI_FORMAT_R32G32B32A32_TYPELESS;

        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
            return DXGI_FORMAT_BC1_TYPELESS;

        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
            return DXGI_FORMAT_BC2_TYPELESS;

        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
            return DXGI_FORMAT_BC3_TYPELESS;

        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            return DXGI_FORMAT_BC4_TYPELESS;

        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
            return DXGI_FORMAT_BC5_TYPELESS;

        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
            return DXGI_FORMAT_BC6H_TYPELESS;

        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            return DXGI_FORMAT_BC7_TYPELESS;
        
        default:
            DXGI_FORMAT_UNKNOWN;
    }

    return DXGI_FORMAT_UNKNOWN;
}

UINT get_sample_count(SampleCount sampleCount)
{
    switch (sampleCount)
    {
        default:
        case SampleCount::UNDEFINED:
            FE_LOG(LogD3D12, ERROR, "Sample count is UNDEFINED.");
        case SampleCount::BIT_1:
            return 1;
        case SampleCount::BIT_2:
            return 2;
        case SampleCount::BIT_4:
            return 4;
        case SampleCount::BIT_8:
            return 8;
        case SampleCount::BIT_16:
            return 16;
        case SampleCount::BIT_32:
            return 32;
        case SampleCount::BIT_64:
            return 64;
    }
}

D3D12_HEAP_TYPE get_heap_type(MemoryUsage memoryUsage)
{
    switch (memoryUsage)
    {
        default:
        case rhi::MemoryUsage::AUTO:
            FE_LOG(LogD3D12, ERROR, "Not recommended to use AUTO memory usage with D3D12! Upload heap will be used.");
        case rhi::MemoryUsage::CPU:
            return D3D12_HEAP_TYPE_UPLOAD;
        case rhi::MemoryUsage::CPU_TO_GPU:
            return D3D12_HEAP_TYPE_GPU_UPLOAD;
        case rhi::MemoryUsage::GPU_TO_CPU:
            return D3D12_HEAP_TYPE_READBACK;
        case rhi::MemoryUsage::GPU:
            return D3D12_HEAP_TYPE_DEFAULT;
    }
}

D3D12_RESOURCE_DIMENSION get_texture_dimension(TextureDimension dimension)
{
    switch (dimension)
    {
        default:
        case TextureDimension::UNDEFINED:
            FE_LOG(LogD3D12, ERROR, "Texture dimension is UNDEFINED.");
            return D3D12_RESOURCE_DIMENSION_UNKNOWN;
        case TextureDimension::TEXTURE1D:
            return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case TextureDimension::TEXTURE2D:
            return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case TextureDimension::TEXTURE3D:
            return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    }
}

D3D12_FILTER get_filter(Filter filter)
{
    switch (filter)
    {
        case Filter::MIN_MAG_MIP_NEAREST:
            return D3D12_FILTER_MIN_MAG_MIP_POINT;
        case Filter::MINIMUM_MIN_MAG_MIP_NEAREST:
            return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
        case Filter::MAXIMUM_MIN_MAG_MIP_NEAREST:
            return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
        case Filter::COMPARISON_MIN_MAG_MIP_NEAREST:
            return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
        case Filter::MIN_MAG_NEAREST_MIP_LINEAR:
            return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        case Filter::MINIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
            return D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
        case Filter::MAXIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
            return D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
        case Filter::COMPARISON_MIN_MAG_NEAREST_MIP_LINEAR:
            return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
        case Filter::MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
            return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case Filter::MINIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
            return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case Filter::MAXIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
            return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case Filter::COMPARISON_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
            return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case Filter::MIN_NEAREST_MAG_MIP_LINEAR:
            return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        case Filter::MINIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
            return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
        case Filter::MAXIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
            return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
        case Filter::COMPARISON_MIN_NEAREST_MAG_MIP_LINEAR:
            return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
        case Filter::MIN_LINEAR_MAG_MIP_NEAREST:
            return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        case Filter::MINIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
            return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
        case Filter::MAXIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
            return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
        case Filter::COMPARISON_MIN_LINEAR_MAG_MIP_NEAREST:
            return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
        case Filter::MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
            return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case Filter::MINIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
            return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case Filter::MAXIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
            return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case Filter::COMPARISON_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
            return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case Filter::MIN_MAG_LINEAR_MIP_NEAREST:
            return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        case Filter::MINIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
            return D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
        case Filter::MAXIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
            return D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
        case Filter::COMPARISON_MIN_MAG_LINEAR_MIP_NEAREST:
            return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        case Filter::MIN_MAG_MIP_LINEAR:
            return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        case Filter::MINIMUM_MIN_MAG_MIP_LINEAR:
            return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
        case Filter::MAXIMUM_MIN_MAG_MIP_LINEAR:
            return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
        case Filter::COMPARISON_MIN_MAG_MIP_LINEAR:
            return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        case Filter::ANISOTROPIC:
            return D3D12_FILTER_ANISOTROPIC;
        case Filter::MINIMUM_ANISOTROPIC:
            return D3D12_FILTER_MINIMUM_ANISOTROPIC;
        case Filter::MAXIMUM_ANISOTROPIC:
            return D3D12_FILTER_MAXIMUM_ANISOTROPIC;
        case Filter::COMPARISON_ANISOTROPIC:
            return D3D12_FILTER_COMPARISON_ANISOTROPIC;
        default:
            FE_LOG(LogD3D12, ERROR, "Sampler filter count is UNDEFINED.");
            return D3D12_FILTER_MIN_MAG_MIP_POINT;
    }
}

D3D12_STATIC_BORDER_COLOR get_border_color(BorderColor borderColor)
{
    switch (borderColor)
    {
        default:
        case BorderColor::UNDEFINED:
            FE_LOG(LogD3D12, ERROR, "Border color is UNDEFINED");
        case BorderColor::FLOAT_TRANSPARENT_BLACK:
        case BorderColor::INT_TRANSPARENT_BLACK:
            return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        case BorderColor::FLOAT_OPAQUE_BLACK:
        case BorderColor::INT_OPAQUE_BLACK:
            return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
        case BorderColor::FLOAT_OPAQUE_WHITE:
        case BorderColor::INT_OPAQUE_WHITE:
            return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    }
}

D3D12_TEXTURE_ADDRESS_MODE get_address_mode(AddressMode addressMode)
{
    switch (addressMode)
    {
        default:
        case AddressMode::UNDEFINED:
            FE_LOG(LogD3D12, ERROR, "Address mode is UNDEFINED.");
        case AddressMode::REPEAT:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case AddressMode::MIRRORED_REPEAT:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case AddressMode::CLAMP_TO_EDGE:
            return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case AddressMode::CLAMP_TO_BORDER:
            return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case AddressMode::MIRROR_CLAMP_TO_EDGE:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
    }
}

D3D12_RESOURCE_STATES get_resource_states(ResourceUsage usage)
{
    D3D12_RESOURCE_STATES states{};

    if (usage == ResourceUsage::UNDEFINED)
    {
        FE_LOG(LogD3D12, ERROR, "Resource usage is UNDEFINED!");
        return states;
    }

    states |= D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

    if (has_flag(usage, ResourceUsage::TRANSFER_SRC))
        states |= D3D12_RESOURCE_STATE_COPY_SOURCE;
    if (has_flag(usage, ResourceUsage::TRANSFER_DST))
        states |= D3D12_RESOURCE_STATE_COPY_DEST;

    if (has_flag(usage, ResourceUsage::STORAGE_BUFFER)
        || has_flag(usage, ResourceUsage::STORAGE_TEXTURE)
        || has_flag(usage, ResourceUsage::STORAGE_TEXEL_BUFFER)
    )
    {
        states |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }

    if (has_flag(usage, ResourceUsage::COLOR_ATTACHMENT)
        || has_flag(usage, ResourceUsage::INPUT_ATTACHMENT)
    )
    {
        states |= D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

    if (has_flag(usage, ResourceUsage::DEPTH_STENCIL_ATTACHMENT))
        states |= D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_DEPTH_WRITE;

    if (has_flag(usage, ResourceUsage::VERTEX_BUFFER))
        states |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

    if (has_flag(usage, ResourceUsage::INDEX_BUFFER))
        states |= D3D12_RESOURCE_STATE_INDEX_BUFFER;

    if (has_flag(usage, ResourceUsage::INDIRECT_BUFFER))
        states |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;

    return states;
}

D3D12_RESOURCE_STATES get_resource_states(ResourceLayout layout)
{
    D3D12_RESOURCE_STATES states{};

    if (layout == ResourceLayout::UNDEFINED)
    {
        FE_LOG(LogD3D12, ERROR, "Resource layout is UNDEFINED!");
        return states;
    }

    if (has_flag(layout, ResourceLayout::TRANSFER_SRC)
        || has_flag(layout, ResourceLayout::MEMORY_READ)   
    )
    {
        states |= D3D12_RESOURCE_STATE_COPY_SOURCE;
    }

    if (has_flag(layout, ResourceLayout::TRANSFER_DST)
        || has_flag(layout, ResourceLayout::MEMORY_WRITE)
    )
    {
        states |= D3D12_RESOURCE_STATE_COPY_DEST;
    }

    if (has_flag(layout, ResourceLayout::GENERAL)
        || has_flag(layout, ResourceLayout::SHADER_WRITE)
    )
    {
        states |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }

    if (has_flag(layout, ResourceLayout::SHADER_READ) ||
        has_flag(layout, ResourceLayout::UNIFORM_BUFFER)
    )
    {
        states |= D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
    }

    if (has_flag(layout, ResourceLayout::COLOR_ATTACHMENT))
    {
        states |= D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

    if (has_flag(layout, ResourceLayout::DEPTH_STENCIL))
    {
        states |= D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_DEPTH_WRITE;
    }
    if (has_flag(layout, ResourceLayout::DEPTH_STENCIL_READ_ONLY))
    {
        states |= D3D12_RESOURCE_STATE_DEPTH_READ;
    }

    if (has_flag(layout, ResourceLayout::VERTEX_BUFFER))
    {
        states |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    }
    if (has_flag(layout, ResourceLayout::INDEX_BUFFER))
    {
        states |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
    }
    if (has_flag(layout, ResourceLayout::INDIRECT_COMMAND_BUFFER))
    {
        states |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    }

    if (has_flag(layout, ResourceLayout::PRESENT_ATTACHMENT))
    {
        states |= D3D12_RESOURCE_STATE_PRESENT;
    }

    return states;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE get_primitive_topology(TopologyType topologyType)
{
    switch (topologyType)
    {
        default:
        case TopologyType::UNDEFINED:
            FE_LOG(LogD3D12, ERROR, "Topology type is UNDEFINED!");
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; 
        case TopologyType::POINT:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        case TopologyType::LINE:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case TopologyType::TRIANGLE:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        case TopologyType::PATCH:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    }
}

D3D12_FILL_MODE get_fill_mode(PolygonMode polygonMode)
{
    switch (polygonMode)
    {
        default:
        case PolygonMode::UNDEFINED:
            FE_LOG(LogD3D12, ERROR, "Polygon mode is UNDEFINED!");
            return D3D12_FILL_MODE_SOLID;
        case PolygonMode::FILL:
            return D3D12_FILL_MODE_SOLID;
        case PolygonMode::LINE:
            return D3D12_FILL_MODE_WIREFRAME;
        case PolygonMode::POINT:
            FE_LOG(LogD3D12, WARNING, "D3D12 does not support POINT polygon mode!");
            return D3D12_FILL_MODE_WIREFRAME;
    }
}

D3D12_CULL_MODE get_cull_mode(CullMode cullMode)
{
    switch (cullMode)
    {
        default:
        case CullMode::UNDEFINED:
            FE_LOG(LogD3D12, ERROR, "Cull mode is UNDEFINED!");
        case CullMode::NONE:
            return D3D12_CULL_MODE_NONE;
        case CullMode::FRONT:
            return D3D12_CULL_MODE_FRONT;
        case CullMode::BACK:
            return D3D12_CULL_MODE_BACK;
        case CullMode::FRONT_AND_BACK:
            FE_LOG(LogD3D12, WARNING, "D3D12 does not support FRONT_AND_BACK cull mode!");
            return D3D12_CULL_MODE_NONE; 
    }
}

BOOL is_front_face_counter_clockwise(FrontFace frontFace)
{
    switch (frontFace)
    {
        default:
        case FrontFace::UNDEFINED:
            FE_LOG(LogD3D12, ERROR, "Front face is UNDEFINED!");
        case FrontFace::COUNTER_CLOCKWISE:
            return TRUE;
        case FrontFace::CLOCKWISE:
            return FALSE;
    }
}

D3D12_LOGIC_OP get_logic_op(LogicOp logicOp)
{
    switch (logicOp)
    {
        default:
        case LogicOp::UNDEFINED:
            FE_LOG(LogD3D12, ERROR, "Logic op is UNDEFINED.");
            return D3D12_LOGIC_OP_NOOP;
        case LogicOp::CLEAR:
            return D3D12_LOGIC_OP_CLEAR;
        case LogicOp::AND:
            return D3D12_LOGIC_OP_AND;
        case LogicOp::AND_REVERSE:
            return D3D12_LOGIC_OP_AND_REVERSE;
        case LogicOp::COPY:
            return D3D12_LOGIC_OP_COPY;
        case LogicOp::AND_INVERTED:
            return D3D12_LOGIC_OP_AND_INVERTED;
        case LogicOp::NO_OP:
            return D3D12_LOGIC_OP_NOOP;
        case LogicOp::XOR:
            return D3D12_LOGIC_OP_XOR;
        case LogicOp::OR:
            return D3D12_LOGIC_OP_OR;
        case LogicOp::NOR:
            return D3D12_LOGIC_OP_NOR;
        case LogicOp::EQUIVALENT:
            return D3D12_LOGIC_OP_EQUIV;
        case LogicOp::INVERT:
            return D3D12_LOGIC_OP_INVERT;
        case LogicOp::OR_REVERSE:
            return D3D12_LOGIC_OP_OR_REVERSE;
        case LogicOp::COPY_INVERTED:
            return D3D12_LOGIC_OP_COPY_INVERTED;
        case LogicOp::OR_INVERTED:
            return D3D12_LOGIC_OP_OR_INVERTED;
        case LogicOp::NAND:
            return D3D12_LOGIC_OP_NAND;
        case LogicOp::SET:
            return D3D12_LOGIC_OP_SET;
    }
}

D3D12_BLEND get_blend_factor(BlendFactor blendFactor)
{
    switch (blendFactor)
    {
        default:
        case BlendFactor::UNDEFINED:
            FE_LOG(LogD3D12, ERROR, "Blend factor is UNDEFINED.");
            return D3D12_BLEND_ZERO;
        case BlendFactor::ZERO:
            return D3D12_BLEND_ZERO;
        case BlendFactor::ONE:
            return D3D12_BLEND_ONE;
        case BlendFactor::SRC_COLOR:
            return D3D12_BLEND_SRC_COLOR;
        case BlendFactor::ONE_MINUS_SRC_COLOR:
            return D3D12_BLEND_INV_SRC_COLOR;
        case BlendFactor::DST_COLOR:
            return D3D12_BLEND_DEST_COLOR;
        case BlendFactor::ONE_MINUS_DST_COLOR:
            return D3D12_BLEND_INV_DEST_COLOR;
        case BlendFactor::SRC_ALPHA:
            return D3D12_BLEND_SRC_ALPHA;
        case BlendFactor::ONE_MINUS_SRC_ALPHA:
            return D3D12_BLEND_INV_SRC_ALPHA;
        case BlendFactor::DST_ALPHA:
            return D3D12_BLEND_DEST_ALPHA;
        case BlendFactor::ONE_MINUS_DST_ALPHA:
            return D3D12_BLEND_INV_DEST_ALPHA;
        case BlendFactor::CONSTANT_COLOR:
            return D3D12_BLEND_BLEND_FACTOR;
        case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
            return D3D12_BLEND_INV_BLEND_FACTOR;
        case BlendFactor::CONSTANT_ALPHA:
            return D3D12_BLEND_ALPHA_FACTOR;
        case BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
            return D3D12_BLEND_INV_ALPHA_FACTOR;
        case BlendFactor::SRC_ALPHA_SATURATE:
            return D3D12_BLEND_ALPHA_FACTOR;
        case BlendFactor::SRC1_COLOR:
            return D3D12_BLEND_SRC1_COLOR;
        case BlendFactor::ONE_MINUS_SRC1_COLOR:
            return D3D12_BLEND_INV_SRC1_COLOR;
        case BlendFactor::SRC1_ALPHA:
            return D3D12_BLEND_SRC1_ALPHA;
        case BlendFactor::ONE_MINUS_SRC1_ALPHA:
            return D3D12_BLEND_INV_SRC1_ALPHA;
    }
}

D3D12_BLEND_OP get_blend_op(BlendOp blendOp)
{
    switch (blendOp)
    {
        default:
        case BlendOp::UNDEFINED:
            FE_LOG(LogD3D12, ERROR, "Blend op is UNDEFINED.");
            return D3D12_BLEND_OP_ADD;
        case BlendOp::ADD:
            return D3D12_BLEND_OP_ADD;
        case BlendOp::SUBTRACT:
            return D3D12_BLEND_OP_SUBTRACT;
        case BlendOp::REVERSE_SUBTRACT:
            return D3D12_BLEND_OP_REV_SUBTRACT;
        case BlendOp::MIN:
            return D3D12_BLEND_OP_MIN;
        case BlendOp::MAX:
            return D3D12_BLEND_OP_MAX;
    }
}

D3D12_COMPARISON_FUNC get_compare_op(CompareOp compareOp)
{
    switch (compareOp)
    {
        default:
        case CompareOp::UNDEFINED:
            FE_LOG(LogD3D12, ERROR, "Compare op is UNDEFINED.");
            return D3D12_COMPARISON_FUNC_NEVER;
        case CompareOp::NEVER:
            return D3D12_COMPARISON_FUNC_NEVER;
        case CompareOp::LESS:
            return D3D12_COMPARISON_FUNC_LESS;
        case CompareOp::EQUAL:
            return D3D12_COMPARISON_FUNC_EQUAL;
        case CompareOp::LESS_OR_EQUAL:
            return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case CompareOp::GREATER:
            return D3D12_COMPARISON_FUNC_GREATER;
        case CompareOp::NOT_EQUAL:
            return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case CompareOp::GREATER_OR_EQUAL:
            return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case CompareOp::ALWAYS:
            return D3D12_COMPARISON_FUNC_ALWAYS;
    }
}

D3D12_STENCIL_OP get_stencil_op(StencilOp stencilOp)
{
    switch (stencilOp)
    {
        default:
        case StencilOp::UNDEFINED:
            FE_LOG(LogD3D12, ERROR, "Stencil op is UNDEFINED.");
            return D3D12_STENCIL_OP_KEEP;
        case StencilOp::KEEP:
            return D3D12_STENCIL_OP_KEEP;
        case StencilOp::ZERO:
            return D3D12_STENCIL_OP_ZERO;
        case StencilOp::REPLACE:
            return D3D12_STENCIL_OP_REPLACE;
        case StencilOp::INCREMENT_AND_CLAMP:
            return D3D12_STENCIL_OP_INCR_SAT;
        case StencilOp::DECREMENT_AND_CLAMP:
            return D3D12_STENCIL_OP_DECR_SAT;
        case StencilOp::INVERT:
            return D3D12_STENCIL_OP_INVERT;
        case StencilOp::INCREMENT_AND_WRAP:
            return D3D12_STENCIL_OP_INCR;
        case StencilOp::DECREMENT_AND_WRAP:
            return D3D12_STENCIL_OP_DECR;
    }
}

D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE get_pass_beginning_access_type(LoadOp loadOp)
{
    switch (loadOp)
    {
        case LoadOp::LOAD:
            return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
        case LoadOp::CLEAR:
            return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
        case LoadOp::DONT_CARE:
            return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
    }
}

D3D12_RENDER_PASS_ENDING_ACCESS_TYPE get_pass_ending_access_type(StoreOp storeOp)
{
    switch (storeOp)
    {
        default:
        case StoreOp::STORE:
            return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
        case StoreOp::DONT_CARE:
            return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
    }
}

DXGI_COLOR_SPACE_TYPE get_color_space(ColorSpace colorSpace)
{
    switch (colorSpace)
    {
    default:
    case (ColorSpace::SRGB):
        return DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
    case (ColorSpace::HDR10_ST2084):
        return DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
    case (ColorSpace::HDR_LINEAR):
        return DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
    }
}


D3D12_SHADER_COMPONENT_MAPPING get_component_swizzle(ComponentSwizzle swizzle)
{
    switch (swizzle)
    {
        case ComponentSwizzle::R:
            return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0;
        case ComponentSwizzle::G:
            return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1;
        case ComponentSwizzle::B:
            return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2;
        case ComponentSwizzle::A:
            return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3;
        case ComponentSwizzle::ONE:
            return D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1;
        case ComponentSwizzle::ZERO:
            return D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0;
        default:
            FE_LOG(LogD3D12, ERROR, "Swizzle is UNDEFINED.");
            return D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0;
    }
}

UINT get_component_mapping(const ComponentMapping& mapping)
{
    return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(
        get_component_swizzle(mapping.r), 
        get_component_swizzle(mapping.g), 
        get_component_swizzle(mapping.b), 
        get_component_swizzle(mapping.a)
    );
}

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ INTERNAL API ]
//============================================================================================================================================================================================

class Device
{
public:
    void init(GPUPreference gpuPreference)
    {
        FE_LOG(LogD3D12, INFO, "Starting D3D12 Device initialization.");

        FE_LOG(LogD3D12, INFO, "D3D12 Device initialization completed.");
    }

private:

} static g_device;

#pragma endregion

void init(const RHIInitInfo* initInfo)
{
    D3D12_HEAP_TYPE type;
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

void update_buffer(Buffer* buffer, uint64 size, const void* data)
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

void create_texture_view(TextureView** textureView, const TextureViewInfo* info)
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

void create_graphics_pipeline(Pipeline** pipeline, const GraphicsPipelineInfo* info)
{

}

void create_compute_pipeline(Pipeline** pipeline, const ComputePipelineInfo* info)
{

}

void create_ray_tracing_pipeline(Pipeline** pipeline, const RayTracingPipelineInfo* info)
{

}

void destroy_pipeline(Pipeline* pipeline)
{

}

void create_acceleration_structure(AccelerationStructure** accelerationStructure, const AccelerationStructureInfo* info)
{

}

void destroy_acceleration_structure(AccelerationStructure* accelerationStructure)
{

}

void write_top_level_acceleration_structure_instance(TLASInstance* instance, void* dst)
{

}

void write_shader_identifier(Pipeline* pipeline, uint32 groupIndex, void* dst)
{

}

void bind_uniform_buffer(Buffer* buffer, uint32 frameIndex, uint32 slot, uint32 size, uint32 offset)
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

void init_texture(CommandBuffer* cmd, Texture* dstTexture, const TextureInitInfo* initInfo)
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

void blit_texture(CommandBuffer* cmd, Texture* srcTexture, Texture* dstTexture, std::array<int32,3> srcOffset, std::array<int32,3> dstOffset, uint32 srcMipLevel, uint32 dstMipLevel, uint32 srcBaseLayer, uint32 dstBaseLayer)
{

}

void set_viewports(CommandBuffer* cmd, const Viewport* viewports, uint32 viewportCount)
{

}

void set_scissors(CommandBuffer* cmd, const Scissor* scissors, uint32 scissorCount)
{

}

void push_constants(CommandBuffer* cmd, Pipeline* pipeline, void* data)
{

}

void bind_vertex_buffer(CommandBuffer* cmd, Buffer* buffer)
{

}

void bind_index_buffer(CommandBuffer* cmd, Buffer* buffer, uint64 offset)
{

}

void bind_pipeline(CommandBuffer* cmd, Pipeline* pipeline)
{

}

void build_acceleration_structure(CommandBuffer* cmd, const AccelerationStructure* dst, const AccelerationStructure* src)
{

}

void begin_rendering(CommandBuffer* cmd, RenderingBeginInfo* beginInfo)
{

}

void end_rendering(CommandBuffer* cmd, SwapChain* swapChain)
{

}

void draw(CommandBuffer* cmd, uint64 vertexCount)
{

}

void draw_indexed(CommandBuffer* cmd, uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 vertexOffset, uint32 firstInstance)
{

}

void draw_indirect(CommandBuffer* cmd, Buffer* buffer, uint32 offset, uint32 drawCount, uint32 stride)
{

}

void draw_indexed_indirect(CommandBuffer* cmd, Buffer* buffer, uint32 offset, uint32 drawCount, uint32 stride)
{

}

void dispatch(CommandBuffer* cmd, uint32 groupCountX, uint32 groupCountY, uint32 groupCountZ)
{

}

void dispatch_mesh(CommandBuffer* cmd, uint32 groupCountX, uint32 groupCountY, uint32 groupCountZ)
{

}

void dispatch_rays(CommandBuffer* cmd, const DispatchRaysInfo* dispatchRaysInfo)
{

}

void add_pipeline_barriers(CommandBuffer* cmd, const PipelineBarrier* barriers, uint32 barriersCount)
{

}

void begin_event(CommandBuffer* cmd, const std::string& eventName)
{

}

void end_event(CommandBuffer* cmd)
{

}

void set_marker(CommandBuffer* cmd, const std::string& markerName)
{

}

void acquire_next_image(SwapChain* swapChain, Semaphore* signalSemaphore, Fence* fence, uint32* frameIndex)
{

}

void submit(const SubmitInfo* submitInfo)
{

}

void present(const PresentInfo* presentInfo)
{

}

void wait_queue_idle(QueueType queueType)
{

}

void wait_for_fences(Fence* const* fences, uint32 fenceCount)
{

}

API get_api()
{
    return API::D3D12;
}

void set_frame_index(uint64 frameIndex)
{

}

void set_name(ResourceVariant resource, const std::string& name)
{

}

uint64 get_min_offset_alignment(const BufferInfo* bufferInfo)
{
    return 0;
}

const GPUProperties& get_gpu_properties()
{
    return {};
}

void fill_function_table()
{
    FE_LOG(LogD3D12, INFO, "Starting D3D12 func table initialization.");

    FE_FILL_RHI_FUNCTION_TABLE();

    FE_LOG(LogD3D12, INFO, "D3D12 func table initialization completed.");
}

}