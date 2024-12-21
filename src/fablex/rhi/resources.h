#pragma once

#include "core/types.h"
#include "core/macro.h"

#ifdef WIN32
#define VULKAN
#include <vulkan.h>
#endif // WIN32

#include <string>
#include <vector>

namespace fe::rhi
{

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
    
    VERTEX = 1 << 0,
    FRAGMENT = 1 << 1,
    TESSELLATION_CONTROL = 1 << 2,
    TESSELLATION_EVALUATION = 1 << 3,
    GEOMETRY = 1 << 4,

    COMPUTE = 1 << 5,

    MESH = 1 << 6,
    TASK = 1 << 7,

    RAY_GENERATION = 1 << 8,
    RAY_INTERSECTION = 1 << 9,
    RAY_ANY_HIT = 1 << 10,
    RAY_CLOSEST_HIT = 1 << 11,
    RAY_MISS = 1 << 12,
    RAY_CALLABLE = 1 << 13
};

enum class ShaderFormat : uint32
{
    UNDEFINED = 0,
    HLSL6,			// For D3D12, DXCompiler
    HLSL_TO_SPIRV,	// For vulkan, DXCompiler
    GLSL_TO_HLSL6,	// For D3D12, compiler will use spirv-cross. This feature will be implemented in the future
    GLSL_TO_SPIRV,	// For vulkan, Shaderc 
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
};

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
    union
    {
#if defined(VULKAN)
        struct
        {
            VkBuffer buffer;
            struct VmaAllocation_T* allocation;
        } vk;
#endif // VULKAN
    };

    uint64 size : 32;
    
    ResourceUsage bufferUsage;
    MemoryUsage memoryUsage : 3;
    ResourceFlags flags : 20;
    Format format : 8;

    void* mappedData = nullptr;
};

FE_COMPILE_CHECK(sizeof(Buffer) == sizeof(uint64) * 8);

struct TextureInfo
{
    uint32 width = 0;
    uint32 height = 0;
    // Only for 3D textures
    uint32 depath = 1;
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
    union
    {
#if defined(VULKAN)
        struct
        {
            VkImage image;
            VmaAllocation_T* allocation;
        } vk;
#endif // VULKAN
    };

    uint32 width : 16;
    uint32 height : 16;
    uint32 depth : 16;
    uint32 mipLevels : 6;
    uint32 layersCount : 10;

    Format format : 8;
    ResourceUsage textureUsage : 24;
    MemoryUsage memoryUsage : 3;
    TextureDimension dimension : 3;
    ResourceFlags resourceFlags : 20;

    void* mappedData;
};

FE_COMPILE_CHECK(sizeof(Texture) == sizeof(uint64) * 8);

struct TextureViewInfo
{
    uint32 baseMipLevel = 0;
    uint32 baseLayer = 0;
    uint32 mipLevels = 1;
    uint32 layerCount = 1;
    // if texture aspect is undefined, rhi will automatically set aspect mask.
    // however, for stencil view it must be set
    TextureAspect aspect = TextureAspect::UNDEFINED;
    ViewType type = ViewType::AUTO;
    ComponentMapping componentMapping;	// If component mapping is not set, mapping from texture will be used
    Format format = Format::UNDEFINED;	// If format is undefined, format from texture will be used
};

struct TextureView
{
    union
    {
#if defined(VULKAN)
        struct
        {
            VkImageView imageView;
        } mVk;
#endif // VULKAN
    };

    Texture* texture;

    uint32 baseMipLevel : 6;
    uint32 baseLayer : 10;
    uint32 mipLevels : 6;
    uint32 layerCount : 10;

    TextureAspect aspect : 3;
    ViewType type : 3;
    Format format : 8;
};

struct BufferViewInfo
{
    uint64 offset = 0;
    uint64 size = 0;
    ViewType type = ViewType::AUTO;
    Format newFormat = Format::UNDEFINED; 
};

struct BufferView
{
    union
    {
#if defined(VULKAN)
        struct
        {
            VkBufferView bufferView;
        };
#endif // VULKAN
    };

    Buffer* buffer = nullptr;

    uint64 offset : 32;
    uint64 size : 32;
    ViewType type;
    Format format;
};

struct SamplerInfo
{
    Filter filter = Filter::UNDEFINED;
    AddressMode addressMode = AddressMode::UNDEFINED;
    BorderColor borderColor = BorderColor::UNDEFINED;
    float minLod = 0.0f;
    float maxLod = 1.0f;
    float maxAnisotropy = 1.0f;
};

struct Sampler
{
    union
    {
#if defined(VULKAN)
        struct
        {
            VkSampler sampler;
        } vk;
#endif // VULKAN
    };
};

struct SwapChainInfo
{
    bool vSync = true;
    uint32 buffersCount = 3;
    uint32 width = 0;
    uint32 height = 0;
    Format format = Format::B8G8R8A8_UNORM;
    ColorSpace colorSpace = ColorSpace::SRGB;
    bool useHDR = false;
};

struct SwapChain
{
    union
    {
#if defined(VULKAN)
        struct
        {
            VkSwapchainKHR swapChain;
            VkSurfaceKHR surface;
        } vk;
#endif // VULKAN
    };
};

struct ShaderInfo
{
    ShaderType shaderType = ShaderType::UNDEFINED;
    uint8_t* data = nullptr;		// Pointer to SPIRV or DXIL data (depends on chosen API)
    uint64_t size = 0;
};

struct Shader
{
    union
    {
#if defined(VULKAN)
        struct
        {
            VkShaderModule shader;
            // TODO: Add reflection data
        } vk;
#endif // VULKAN
    };

    ShaderType type{ShaderType::UNDEFINED};
};

enum class TopologyType
{
    UNDEFINED,
    POINT,
    LINE,
    TRIANGLE,
    PATCH,
};

struct AssemblyState
{
    TopologyType topologyType = TopologyType::UNDEFINED;
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

struct RasterizationState
{
    PolygonMode polygonMode = PolygonMode::UNDEFINED;
    CullMode cullMode = CullMode::UNDEFINED;
    FrontFace frontFace = FrontFace::UNDEFINED;
    bool isBiasEnabled;
    float lineWidth = 1.0f;
};

struct VertexBindingDescription
{
    uint32_t binding;
    uint32_t stride;
};

struct VertexAttributeDescription
{
    uint32_t binding;
    uint32_t location;
    uint32_t offset;
    Format format = Format::UNDEFINED;
};

struct MultisampleState
{
    SampleCount sampleCount = SampleCount::UNDEFINED;
    bool isEnabled;
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

struct ColorBlendAttachmentState
{
    bool isBlendEnabled;
    BlendFactor srcColorBlendFactor = BlendFactor::UNDEFINED;
    BlendFactor dstColorBlendFactor = BlendFactor::UNDEFINED;
    BlendOp colorBlendOp = BlendOp::UNDEFINED;
    BlendFactor srcAlphaBlendFactor = BlendFactor::UNDEFINED;
    BlendFactor dstAlphaBlendFactor = BlendFactor::UNDEFINED;
    BlendOp alphaBlendOp = BlendOp::UNDEFINED;
    uint64_t colorWriteMask = 0xF;
};

struct ColorBlendState
{
    bool isLogicOpEnabled;
    LogicOp logicOp{ LogicOp::UNDEFINED };
    std::vector<ColorBlendAttachmentState> colorBlendAttachments;
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

struct StencilOpState
{
    StencilOp failOp = StencilOp::UNDEFINED;
    StencilOp passOp = StencilOp::UNDEFINED;
    StencilOp depthFailOp = StencilOp::UNDEFINED;
    CompareOp compareOp = CompareOp::UNDEFINED;
    uint32_t compareMask;
    uint32_t writeMask;
    uint32_t reference;
};

struct DepthStencilState
{
    bool isDepthTestEnabled;
    bool isDepthWriteEnabled;
    CompareOp compareOp = CompareOp::UNDEFINED;
    bool isStencilTestEnabled;
    StencilOpState frontStencil;
    StencilOpState backStencil;
};

struct GraphicsPipelineInfo
{
    AssemblyState assemblyState;
    RasterizationState rasterizationState;
    MultisampleState multisampleState;
    ColorBlendState colorBlendState;
    DepthStencilState depthStencilState;
    std::vector<Shader*> shaderStages;
    std::vector<VertexBindingDescription> bindingDescriptions;
    std::vector<VertexAttributeDescription> attributeDescriptions;
    std::vector<Format> colorAttachmentFormats;
    Format depthFormat = Format::UNDEFINED;
};

struct ComputePipelineInfo
{
    Shader shaderStage;
};

enum class PipelineType : uint32
{
    UNDEFINED,
    GRAPHICS,
    COMPUTE,
    RAY_TRACING
};

struct Pipeline
{
    union
    {
#if defined (VULKAN)
        struct
        {
            // TODO: Add pipeline layout
            VkPipeline pipeline;
        } vk;
#endif
    };

    PipelineType type{PipelineType::UNDEFINED};
};

struct CommandPoolInfo
{
    QueueType queueType;
};

struct CommandPool
{
    union
    {
#if defined(VULKAN)
        struct
        {
            VkCommandPool cmdPool;
        } vk;
#endif
    };

    QueueType queueType;
};

struct CommandBufferInfo
{
    CommandPool* cmdPool = nullptr;
};

struct CommandBuffer
{
    union
    {
#if defined(VULKAN)
        struct
        {
            VkCommandBuffer cmdBuffer;
        } vk;
#endif
    };
};

class PipelineBarrier
{
public:
    enum class BarrierType
    {
        MEMORY,
        BUFFER,
        TEXTURE
    };

    struct MemoryBarrier
    {
        ResourceLayout srcLayout;
        ResourceLayout dstLayout;
    };

    struct BufferBarrier
    {
        const Buffer* buffer;
        ResourceLayout srcLayout;
        ResourceLayout dstLayout;
    };

    struct TextureBarrier
    {
        const Texture* texture;
        ResourceLayout srcLayout;
        ResourceLayout dstLayout;
        uint32_t levelCount;
        uint32_t baseMipLevel;
        uint32_t layerCount;
        uint32_t baseLayer;
    };

    PipelineBarrier() = default;
    PipelineBarrier(const MemoryBarrier& memoryBarrier) { m_barrier.memoryBarrier = memoryBarrier; }
    PipelineBarrier(const BufferBarrier& bufferBarrier) { m_barrier.bufferBarrier = bufferBarrier; }
    PipelineBarrier(const TextureBarrier& textureBarrier) { m_barrier.textureBarrier = textureBarrier; }

    PipelineBarrier(ResourceLayout srcLayout, ResourceLayout dstLayout)
    { 
        m_barrier.memoryBarrier = MemoryBarrier{srcLayout, dstLayout}; 
    }

    PipelineBarrier(Buffer* buffer, ResourceLayout srcLayout, ResourceLayout dstLayout)
    { 
        m_barrier.bufferBarrier = BufferBarrier{buffer, srcLayout, dstLayout};
    }

    PipelineBarrier(Texture* texture,
        ResourceLayout srcLayout,
        ResourceLayout dstLayout,
        uint32_t baseMipLevel = 0,
        uint32_t levelCount = 0,
        uint32_t baseLayer = 0,
        uint32_t layerCount = 0)
    { 
        m_barrier.textureBarrier = TextureBarrier{texture, srcLayout, dstLayout, levelCount, baseMipLevel, layerCount, baseLayer};
    }

    void set_memory_barrier(ResourceLayout srcLayout, ResourceLayout dstLayout)
    {
        m_barrier.memoryBarrier = MemoryBarrier{srcLayout, dstLayout};
    }

    void set_buffer_barrier(Buffer* buffer, ResourceLayout srcLayout, ResourceLayout dstLayout)
    {
        m_barrier.bufferBarrier = BufferBarrier{buffer, srcLayout, dstLayout};
    }

    void set_texture_barrier(Texture* texture,
        ResourceLayout srcLayout,
        ResourceLayout dstLayout,
        uint32_t baseMipLevel = 0,
        uint32_t levelCount = 0,
        uint32_t baseLayer = 0,
        uint32_t layerCount = 0)
    {
        m_barrier.textureBarrier = TextureBarrier{texture, srcLayout, dstLayout, levelCount, baseMipLevel, layerCount, baseLayer};
    }

    const MemoryBarrier* get_memory_barrier() const { return &m_barrier.memoryBarrier; }
    const BufferBarrier* get_buffer_barrier() const { return &m_barrier.bufferBarrier; }
    const TextureBarrier* get_texture_barrier() const { return &m_barrier.textureBarrier; }

private:
    union
    {
        MemoryBarrier memoryBarrier;
        BufferBarrier bufferBarrier;
        TextureBarrier textureBarrier;
    } m_barrier;
};

struct Semaphore
{
    union
    {
#if defined(VULKAN)
        struct
        {
            VkSemaphore semaphore;
        } vk;
#endif
    };
};

struct Fence
{
    union
    {
#if defined(VULKAN)
        struct
        {
            VkFence fence;
        } vk;
#endif
    };
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

struct ClearValues
{
    float color[4];

    struct
    {
        float depth;
        uint32_t stencil;
    } depthStencil;
};

struct RenderTarget
{
    TextureView* target;
    LoadOp loadOp;
    StoreOp storeOp;
    ClearValues clearValue;
};

enum class RenderingBeginInfoFlags : uint32
{
    SUSPENDING = 1 << 0,
    RESUMING = 1 << 1
};

struct MultiviewInfo
{
    bool isEnabled = false; // false is default
    uint32_t viewCount = 0;
};

struct RenderingBeginInfo
{
    enum Type
    {
        OFFSCREEN_PASS,
        SWAP_CHAIN_PASS,
    };

    Type type;

    struct OffscreenPass
    {
        std::vector<RenderTarget> renderTargets;
        RenderingBeginInfoFlags flags;
        MultiviewInfo multiviewInfo;		// Not necessary

        ~OffscreenPass() = default;
    };

    struct SwapChainPass
    {
        SwapChain* swapChain = nullptr;
        ClearValues clearValues;

        ~SwapChainPass() = default;
    };

    union
    {
        OffscreenPass offscreenPass;
        SwapChainPass swapChainPass;
    };

    ~RenderingBeginInfo() { }
};

struct SubmitInfo
{
    std::vector<CommandBuffer*> cmdBuffers;
    std::vector<Fence*> signalFences;
    std::vector<Semaphore> waitSemaphores;
    std::vector<Semaphore*> signalSemaphores;
    bool isSubmitDone = false;
};

struct PresentInfo
{
    SwapChain* swapChain = nullptr;
    std::vector<Semaphore*> waitSemaphores;
    bool isSubmitDone = false;
};

struct Viewport
{
    uint32 x = 0;
    uint32 y = 0;
    uint32 width;
    uint32 height;
    uint32 minDepth = 0;
    uint32 maxDepth = 1;
};

struct Scissor
{
    int32 left = 0;
    int32 top = 0;
    int32 right = 0;
    int32 bottom = 0;
};

struct QueryPoolInfo
{
    // TODO
};

struct QueryPool
{
    // TODO
};

struct GPUMemoryUsage
{
    uint64_t total = 0;
    uint64_t usage = 0;
};

struct GPUProperties
{
    uint32 bufferCount = 0;
    ValidationMode validationMode = ValidationMode::DISABLED;
    GPUCapability capabilities = GPUCapability::UNDEFINED;
    GPUType gpuType = GPUType::DISCRETE;
    uint64 shaderIdentifierSize = 0;
    uint64 accelerationStructureInstanceSize = 0;
    uint64 timestampFrequency = 0;
    uint32 vendorID = 0;
    uint32 deviceID = 0;
    std::string gpuName;
    std::string driverDescription;
};

}