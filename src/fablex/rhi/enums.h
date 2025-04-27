#pragma once

#include "core/types.h"
#include "core/flags_operations.h"

namespace fe::rhi
{

enum class API
{
    VK,
    D3D12
};

enum class GPUCapability : uint32
{
    UNDEFINED = 0,
    TESSELLATION = 1 << 0,
    SAMPLER_MINMAX = 1 << 1,
    VARIABLE_RATE_SHADING = 1 << 2,
    VARIABLE_RATE_SHADING_TIER2 = 1 << 3,
    MESH_SHADER = 1 << 4,
    RAY_TRACING = 1 << 5,
    SPARSE_BUFFER = 1 << 6,
    SPARSE_TEXTURE2D = 1 << 7,
    SPARSE_TEXTURE3D = 1 << 8,
    SPARSE_NULL_MAPPING = 1 << 9,
    SPARSE_TILE_POOL = 1 << 10,
    FRAGMENT_SHADER_INTERLOCK = 1 << 11,
    CACHE_COHERENT_UMA = 1 << 12
};

enum class GPUPreference : uint32
{
    INTEGRATED,
    DISCRETE
};

enum class GPUType : uint32
{
    OTHER,
    INTEGRATED,
    DISCRETE,
    VIRTUAL
};

enum class ValidationMode : uint32
{
    DISABLED,
    ENABLED,
    GPU,
    VERBOSE
};

enum class ResourceFlags : uint32
{
    UNDEFINED = 0,
    CUBE_TEXTURE = 1 << 0,
    RAW_BUFFER = 1 << 1,
    STRUCTURED_BUFFER = 1 << 2,
    RAY_TRACING = 1 << 3,
};

enum class LogicOp : uint32
{
    UNDEFINED,
    CLEAR,
    AND,
    AND_REVERSE,
    COPY,
    AND_INVERTED,
    NO_OP,
    XOR,
    OR,
    NOR,
    EQUIVALENT,
    INVERT,
    OR_REVERSE,
    COPY_INVERTED,
    OR_INVERTED,
    NAND,
    SET,
};

enum class ResourceLayout : uint32
{
    UNDEFINED = 1 << 0,
    GENERAL = 1 << 1,
    SHADER_READ = 1 << 2,
    SHADER_WRITE = 1 << 3,
    MEMORY_READ = 1 << 4,
    MEMORY_WRITE = 1 << 5,
    TRANSFER_SRC = 1 << 6,
    TRANSFER_DST = 1 << 7,

    COLOR_ATTACHMENT = 1 << 8,
    DEPTH_STENCIL = 1 << 9,
    DEPTH_STENCIL_READ_ONLY = 1 << 10,

    INDIRECT_COMMAND_BUFFER = 1 << 11,
    VERTEX_BUFFER = 1 << 12,
    INDEX_BUFFER = 1 << 13,
    UNIFORM_BUFFER = 1 << 14,

    PRESENT_ATTACHMENT = 1 << 15
};

enum class ResourceUsage : uint32
{
    UNDEFINED = 1 << 0,
    TRANSFER_SRC = 1 << 1,
    TRANSFER_DST = 1 << 2,
    SAMPLED_TEXTURE = 1 << 3,
    STORAGE_TEXTURE = 1 << 4,
    COLOR_ATTACHMENT = 1 << 5,
    DEPTH_STENCIL_ATTACHMENT = 1 << 6,
    TRANSIENT_ATTACHMENT = 1 << 7,
    INPUT_ATTACHMENT = 1 << 8,
    UNIFORM_TEXEL_BUFFER = 1 << 10,
    STORAGE_TEXEL_BUFFER = 1 << 11,
    UNIFORM_BUFFER = 1 << 12,
    STORAGE_BUFFER = 1 << 13,
    INDEX_BUFFER = 1 << 14,
    VERTEX_BUFFER = 1 << 15,
    INDIRECT_BUFFER = 1 << 16
};

enum class MemoryUsage : uint32
{
    AUTO,
    CPU,
    GPU,
    CPU_TO_GPU,
    GPU_TO_CPU
};

enum class Format : uint32
{
    UNDEFINED = 0,
    R4G4_UNORM,
    R4G4B4A4_UNORM,
    B4G4R4A4_UNORM,
    R5G5B5A1_UNORM,
    B5G5R5A1_UNORM,
    A1R5G5B5_UNORM,

    R8_UNORM,
    R8_SNORM,
    R8_UINT,
    R8_SINT,
    R8_SRGB,

    R8G8_UNORM,
    R8G8_SNORM,
    R8G8_UINT,
    R8G8_SINT,
    R8G8_SRGB,

    R8G8B8A8_UNORM,
    R8G8B8A8_SNORM,
    R8G8B8A8_UINT,
    R8G8B8A8_SINT,
    R8G8B8A8_SRGB,

    B8G8R8A8_SRGB,
    B8G8R8A8_UNORM,
    B8G8R8A8_SNORM,

    R10G10B10A2_UNORM,
    R10G10B10A2_SNORM,

    R16_UNORM,
    R16_SNORM,
    R16_UINT,
    R16_SINT,
    R16_SFLOAT,

    R16G16_UNORM,
    R16G16_SNORM,
    R16G16_UINT,
    R16G16_SINT,
    R16G16_SFLOAT,

    R16G16B16A16_UNORM,
    R16G16B16A16_SNORM,
    R16G16B16A16_UINT,
    R16G16B16A16_SINT,
    R16G16B16A16_SFLOAT,

    R32_UINT,
    R32_SINT,
    R32_SFLOAT,
    R32G32_UINT,
    R32G32_SINT,
    R32G32_SFLOAT,

    R32G32B32_UINT,
    R32G32B32_SINT,
    R32G32B32_SFLOAT,
    
    R32G32B32A32_UINT,
    R32G32B32A32_SINT,
    R32G32B32A32_SFLOAT,

    D16_UNORM,
    D32_SFLOAT,

    S8_UINT,
    D16_UNORM_S8_UINT,
    D24_UNORM_S8_UINT,
    D32_SFLOAT_S8_UINT,

    BC1_RGBA_UNORM,
    BC1_RGBA_SRGB_UNORM,
    BC2_UNORM,
    BC2_SRGB,
    BC3_UNORM,
    BC3_SRGB,
    BC4_UNORM,
    BC4_SNORM,
    BC5_UNORM,
    BC5_SNORM,
    BC6H_UFLOAT,
    BC6H_SFLOAT,
    BC7_UNORM,
    BC7_SRGB
};

enum class AddressMode : uint32
{
    UNDEFINED,
    REPEAT,
    MIRRORED_REPEAT,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER,
    MIRROR_CLAMP_TO_EDGE
};

// Based on D3D12
enum class Filter : uint32
{
    UNDEFINED,
    MIN_MAG_MIP_NEAREST,
    MIN_MAG_NEAREST_MIP_LINEAR,
    MIN_NEAREST_MAG_LINEAR_MIP_NEAREST,
    MIN_NEAREST_MAG_MIP_LINEAR,
    MIN_LINEAR_MAG_MIP_NEAREST,
    MIN_LINEAR_MAG_NEAREST_MIP_LINEAR,
    MIN_MAG_LINEAR_MIP_NEAREST,
    MIN_MAG_MIP_LINEAR,
    ANISOTROPIC,
    COMPARISON_MIN_MAG_MIP_NEAREST,
    COMPARISON_MIN_MAG_NEAREST_MIP_LINEAR,
    COMPARISON_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST,
    COMPARISON_MIN_NEAREST_MAG_MIP_LINEAR,
    COMPARISON_MIN_LINEAR_MAG_MIP_NEAREST,
    COMPARISON_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR,
    COMPARISON_MIN_MAG_LINEAR_MIP_NEAREST,
    COMPARISON_MIN_MAG_MIP_LINEAR,
    COMPARISON_ANISOTROPIC,
    MINIMUM_MIN_MAG_MIP_NEAREST,
    MINIMUM_MIN_MAG_NEAREST_MIP_LINEAR,
    MINIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST,
    MINIMUM_MIN_NEAREST_MAG_MIP_LINEAR,
    MINIMUM_MIN_LINEAR_MAG_MIP_NEAREST,
    MINIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR,
    MINIMUM_MIN_MAG_LINEAR_MIP_NEAREST,
    MINIMUM_MIN_MAG_MIP_LINEAR,
    MINIMUM_ANISOTROPIC,
    MAXIMUM_MIN_MAG_MIP_NEAREST,
    MAXIMUM_MIN_MAG_NEAREST_MIP_LINEAR,
    MAXIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST,
    MAXIMUM_MIN_NEAREST_MAG_MIP_LINEAR,
    MAXIMUM_MIN_LINEAR_MAG_MIP_NEAREST,
    MAXIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR,
    MAXIMUM_MIN_MAG_LINEAR_MIP_NEAREST,
    MAXIMUM_MIN_MAG_MIP_LINEAR,
    MAXIMUM_ANISOTROPIC
};

enum class SampleCount : uint32
{
    UNDEFINED,
    BIT_1,
    BIT_2,
    BIT_4,
    BIT_8,
    BIT_16,
    BIT_32,
    BIT_64
};

enum class TextureDimension : uint32
{
    UNDEFINED,
    TEXTURE1D,
    TEXTURE2D,
    TEXTURE3D
};

enum class TextureAspect : uint32
{
    UNDEFINED,
    COLOR,
    DEPTH,
    STENCIL
};

enum class BorderColor : uint32
{
    UNDEFINED,
    FLOAT_TRANSPARENT_BLACK,
    INT_TRANSPARENT_BLACK,
    FLOAT_OPAQUE_BLACK,
    INT_OPAQUE_BLACK,
    FLOAT_OPAQUE_WHITE,
    INT_OPAQUE_WHITE
};

enum class ShaderType : uint32
{
    UNDEFINED = 0,
    
    VERTEX,
    FRAGMENT,
    TESSELLATION_CONTROL,
    TESSELLATION_EVALUATION,
    GEOMETRY,

    COMPUTE,

    MESH,
    TASK,

    RAY_GENERATION,
    RAY_INTERSECTION,
    RAY_ANY_HIT,
    RAY_CLOSEST_HIT,
    RAY_MISS,
    RAY_CALLABLE,

    LIB
};

enum class ShaderFormat : uint32
{
    UNDEFINED = 0,
    HLSL6,			// For D3D12, DXCompiler
    HLSL_TO_SPIRV,	// For vulkan, DXCompiler
};

enum class HLSLShaderModel : uint32
{
    SM_6_0,
    SM_6_1,
    SM_6_2,
    SM_6_3,
    SM_6_4,
    SM_6_5,
    SM_6_6,
    SM_6_7
};

enum class ColorSpace : uint32
{
    SRGB,
    HDR10_ST2084,
    HDR_LINEAR
};

enum class ViewType : uint32
{
    // View type will be defined according to the texture or buffer usage and flags.
    // Not recommended to use for release versions because it can lead to
    // overwriting descriptors and undefined behavior
    AUTO,
    SRV,	// shader resource view
    UAV,	// unordered access view
    RTV,	// render target view
    DSV,	// depth stencil view
};

enum class ComponentSwizzle : uint32
{
    UNDEFINED,
    ZERO,
    ONE,
    R,
    G,
    B,
    A
};

struct ComponentMapping
{
    ComponentSwizzle r = ComponentSwizzle::UNDEFINED;
    ComponentSwizzle g = ComponentSwizzle::UNDEFINED;
    ComponentSwizzle b = ComponentSwizzle::UNDEFINED;
    ComponentSwizzle a = ComponentSwizzle::UNDEFINED;
};

enum class QueueType : uint32
{
    GRAPHICS,
    COMPUTE,
    TRANSFER,

    COUNT
};

enum class TopologyType
{
    UNDEFINED,
    POINT,
    LINE,
    TRIANGLE,
    PATCH,
};

/**
* POLYGON_MODE_POINT available only if you use Vulkan
*/
enum class PolygonMode
{
    UNDEFINED,
    FILL,
    LINE,
    POINT
};

/**
    * CULL_MODE_FRONT_AND_BACK available only if you use Vulkan
    */
enum class CullMode
{
    UNDEFINED,
    NONE,
    FRONT,
    BACK,
    FRONT_AND_BACK
};

enum class FrontFace
{
    UNDEFINED,
    CLOCKWISE,
    COUNTER_CLOCKWISE
};

enum class BlendFactor
{
    UNDEFINED,
    ZERO,
    ONE,
    SRC_COLOR,
    ONE_MINUS_SRC_COLOR,
    DST_COLOR,
    ONE_MINUS_DST_COLOR,
    SRC_ALPHA,
    ONE_MINUS_SRC_ALPHA,
    DST_ALPHA,
    ONE_MINUS_DST_ALPHA,
    CONSTANT_COLOR,
    ONE_MINUS_CONSTANT_COLOR,
    CONSTANT_ALPHA,
    ONE_MINUS_CONSTANT_ALPHA,
    SRC_ALPHA_SATURATE,
    SRC1_COLOR,
    ONE_MINUS_SRC1_COLOR,
    SRC1_ALPHA,
    ONE_MINUS_SRC1_ALPHA,
};

enum class BlendOp
{
    UNDEFINED,
    ADD,
    SUBTRACT,
    REVERSE_SUBTRACT,
    MIN,
    MAX,
};

enum class CompareOp
{
    UNDEFINED,
    NEVER,
    LESS,
    EQUAL,
    LESS_OR_EQUAL,
    GREATER,
    NOT_EQUAL,
    GREATER_OR_EQUAL,
    ALWAYS,
};

enum class StencilOp
{
    UNDEFINED,
    KEEP,
    ZERO,
    REPLACE,
    INCREMENT_AND_CLAMP,
    DECREMENT_AND_CLAMP,
    INVERT,
    INCREMENT_AND_WRAP,
    DECREMENT_AND_WRAP,
};

enum class PipelineType : uint32
{
    UNDEFINED,
    GRAPHICS,
    COMPUTE,
    RAY_TRACING
};

enum class LoadOp
{
    LOAD,
    CLEAR,
    DONT_CARE,
};

enum class StoreOp
{
    STORE,
    DONT_CARE,
};

enum class RenderingBeginInfoFlags : uint32
{
    SUSPENDING = 1 << 0,
    RESUMING = 1 << 1
};

}

ENABLE_BIT_MASK(fe::rhi::GPUCapability)
ENABLE_BIT_MASK(fe::rhi::ResourceUsage)
ENABLE_BIT_MASK(fe::rhi::ResourceFlags)
ENABLE_BIT_MASK(fe::rhi::ResourceLayout)
ENABLE_BIT_MASK(fe::rhi::RenderingBeginInfoFlags)
