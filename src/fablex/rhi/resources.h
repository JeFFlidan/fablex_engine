#pragma once

#include "enums.h"
#include "core/types.h"
#include "core/macro.h"
#include "core/window.h"
#include "core/flags_operations.h"

#ifdef WIN32
#define FE_VULKAN
#include <vulkan/vulkan.h>
#endif // WIN32

#include <string>
#include <vector>
#include <array>
#include <variant>

struct VmaAllocation_T;

namespace fe::rhi
{

#define FE_DEFINE_RHI_RESOURCE()    \
private:\
    std::variant<   \
        std::monostate, \
        Vulkan    \
    > m_apiData;\
public:\
    Vulkan& vk() { return *std::get_if<Vulkan>(&m_apiData); } \
    const Vulkan& vk() const { return *std::get_if<Vulkan>(&m_apiData); }   \
    void init_vk() { m_apiData.emplace<Vulkan>(); }

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

struct MipMap
{
    uint32 offset = 0;
    uint32 layer = 0;
};

struct TextureInitInfo
{
    rhi::Buffer* buffer;
    std::vector<MipMap> mipMaps;
};

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

struct BufferViewInfo
{
    uint64 offset = 0;
    uint64 size = 0;
    ViewType type = ViewType::AUTO;
    Format newFormat = Format::UNDEFINED; 
};

struct BufferView
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkBufferView bufferView = VK_NULL_HANDLE;
#endif // FE_VULKAN
    };

    FE_DEFINE_RHI_RESOURCE()

    const Buffer* buffer = nullptr;

    uint64 descriptorIndex : 32;
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
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkSampler sampler = VK_NULL_HANDLE;        
#endif // FE_VULKAN
    };

    FE_DEFINE_RHI_RESOURCE()

    uint32 descriptorIndex;
};

struct SwapChainInfo
{
    bool vSync = true;
    uint32 bufferCount = 3;
    Window* window = nullptr;
    Format format = Format::B8G8R8A8_UNORM;
    ColorSpace colorSpace = ColorSpace::SRGB;
    bool useHDR = false;
};

struct SwapChain
{
    struct Vulkan
    {
#if defined(FE_VULKAN) 
        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        uint32 imageIndex;
#endif // FE_VULKAN
    };

    FE_DEFINE_RHI_RESOURCE()

    Window* window = nullptr;
    ColorSpace colorSpace;
    Format format = Format::UNDEFINED;
    uint32 bufferCount = 0;
    bool vSync = true;
};

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

struct ShaderLibrary
{
    rhi::Shader* shader = nullptr;
    rhi::ShaderType type = rhi::ShaderType::RAY_GENERATION;
    std::string entryPoint;
};

struct AssemblyState
{
    TopologyType topologyType = TopologyType::UNDEFINED;
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
    Shader* shaderStage;
};

struct ShaderHitGroup
{
    enum Type
    {
        GENERAL, // raygen or miss
        TRIANGLES,
        PROCEDURAL
    };

    static constexpr uint32 s_invalidIndex = ~0u;

    Type type = GENERAL;
    ShaderType shaderType = ShaderType::UNDEFINED;
    std::string name;
    uint32 generalShader = s_invalidIndex;
    uint32 closestHitShader = s_invalidIndex;
    uint32 anyHitShader = s_invalidIndex;
    uint32 intersectionShader = s_invalidIndex;
};

struct RayTracingPipelineInfo
{
    std::vector<ShaderLibrary> shaderLibraries;
    std::vector<ShaderHitGroup> shaderHitGroups;
    uint32 maxTraceDepthRecursion = 1;
    uint32 maxAttributeSizeInBytes = 0;
    uint32 maxPayloadSizeInBytes = 0;
};

struct Pipeline
{
    struct Vulkan
    {
#if defined (FE_VULKAN)
        VkPipeline pipeline = VK_NULL_HANDLE;
        uint64 layoutHash = 0;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()

    PipelineType type{PipelineType::UNDEFINED};
};

struct BLAS
{
    struct Geometry
    {
        enum class Flags
        {
            UNDEFINED = 0,
            OPAQUE = 1 << 0,
            NO_DUPLICATE_ANYHIT_INVOCATION = 1 << 1,
            USE_TRANSFORM = 1 << 2
        };

        enum Type
        {
            TRIANGLES,
            PROCEDURAL_AABBS
        };

        struct Triangles
        {
            Buffer* vertexBuffer = nullptr;
            Buffer* indexBuffer = nullptr;
            uint32 indexCount = 0;
            uint32 indexOffset = 0;
            uint32 vertexCount = 0;
            uint32 vertexOffset = 0;
            uint32 vertexStride = 0;
            rhi::Format vertexFormat = rhi::Format::R32G32B32_SFLOAT;
            Buffer* transform3x4Buffer = nullptr;
            uint32 transform3x4BufferOffset = 0;
        };

        struct ProceduralAABBs
        {
            Buffer* aabbBuffer = nullptr;
            uint32 ofsset = 0;
            uint32 count = 0;
            uint32 stride = 0;
        };

        Flags flags = Flags::UNDEFINED;
        Type type = TRIANGLES;

        Triangles triangles;
        ProceduralAABBs aabbs;
    };

    std::vector<Geometry> geometries;
};

struct AccelerationStructure;

struct TLAS
{
    struct Instance
    {
        enum class Flags
        {
            UNDEFINED = 0,
            TRIANGLE_CULL_DISABLE = 1 << 0,
            TRIANGLE_FRONT_COUNTERCLOCKWISE = 1 << 1,
            FORCE_OPAQUE = 1 << 2,
            FORCE_NON_OPAQUE = 1 << 3
        };

        Flags flags = Flags::UNDEFINED;
        float transform[3][4];
        uint32 instanceID = 0;
        uint32 instanceMask = 0;
        uint32 instanceContributionToHitGroupIndex = 0;
        AccelerationStructure* blas = nullptr;
    };

    Buffer* instanceBuffer = nullptr;
    uint32 offset = 0;
    uint32 count = 0;
};

struct AccelerationStructureInfo
{
    enum class Flags
    {
        UNDEFINED = 0,
        ALLOW_UPDATE = 1 << 0,
        ALLOW_COMPACTION = 1 << 1,
        PREFER_FAST_TRACE = 1 << 2,
        PREFER_FAST_BUILD = 1 << 3,
        LOW_MEMORY = 1 << 4
    };

    enum Type
    {
        BOTTOM_LEVEL,
        TOP_LEVEL
    };
    
    Flags flags = Flags::UNDEFINED;
    Type type = Type::BOTTOM_LEVEL;

    BLAS blas;
    TLAS tlas;
};

struct AccelerationStructure
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VmaAllocation_T* allocation = nullptr;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkAccelerationStructureKHR accelerationStructure = VK_NULL_HANDLE;
        
        VkDeviceAddress scratchAddress = 0;
        VkDeviceAddress accelerationStructureAddress = 0;
#endif // FE_VULKAN
    };

    FE_DEFINE_RHI_RESOURCE()

    AccelerationStructureInfo info;
    uint64 size = 0;
    uint32 descriptorIndex; 
};

struct CommandPoolInfo
{
    QueueType queueType;
};

struct CommandPool
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkCommandPool cmdPool = VK_NULL_HANDLE;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()

    QueueType queueType;
};

struct CommandBufferInfo
{
    CommandPool* cmdPool = nullptr;
};

struct CommandBuffer
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()

    const CommandPool* cmdPool = nullptr;
};

class PipelineBarrier
{
public:
    enum BarrierType
    {
        MEMORY,
        BUFFER,
        TEXTURE
    } type;

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
        type = MEMORY;
    }

    PipelineBarrier(Buffer* buffer, ResourceLayout srcLayout, ResourceLayout dstLayout)
    { 
        m_barrier.bufferBarrier = BufferBarrier{buffer, srcLayout, dstLayout};
        type = BUFFER;
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
        type = TEXTURE;
    }

    void set_memory_barrier(ResourceLayout srcLayout, ResourceLayout dstLayout)
    {
        m_barrier.memoryBarrier = MemoryBarrier{srcLayout, dstLayout};
        type = MEMORY;
    }

    void set_buffer_barrier(Buffer* buffer, ResourceLayout srcLayout, ResourceLayout dstLayout)
    {
        m_barrier.bufferBarrier = BufferBarrier{buffer, srcLayout, dstLayout};
        type = BUFFER;
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
        type = TEXTURE;
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
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkSemaphore semaphore = VK_NULL_HANDLE;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()
};

struct Fence
{
    struct Vulkan
    {
#if defined(FE_VULKAN)
        VkFence fence = VK_NULL_HANDLE;
#endif
    };

    FE_DEFINE_RHI_RESOURCE()
};

struct ClearValues
{
    std::array<float, 4> color = { 0.0f, 0.0f, 0.0f, 1.0f };

    struct
    {
        float depth = 1.0f;
        uint32 stencil = 0;
    } depthStencil;
};

struct RenderTarget
{
    TextureView* target;
    LoadOp loadOp;
    StoreOp storeOp;
    ClearValues clearValue;
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

    RenderingBeginInfo(Type inType) : type(inType) { }

    Type type;
    MultiviewInfo multiviewInfo;
    RenderingBeginInfoFlags flags;

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

    OffscreenPass offscreenPass;
    SwapChainPass swapChainPass;
};

struct SubmitInfo
{
    QueueType queueType = QueueType::GRAPHICS;
    std::vector<CommandBuffer*> cmdBuffers;
    std::vector<Semaphore*> waitSemaphores;
    std::vector<Semaphore*> signalSemaphores;

    void clear()
    {
        cmdBuffers.clear();
        waitSemaphores.clear();
        signalSemaphores.clear();
    }
};

struct PresentInfo
{
    std::vector<SwapChain*> swapChains;
    std::vector<Semaphore*> waitSemaphores;
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

struct ShaderIdentifierBuffer
{
    enum Type
    {
        UNDEFINED,
        RAY_GENERATION,
        RAY_MISS,
        RAY_HIT,
        RAY_CALLABLE
    };

    Type type = UNDEFINED;
    std::string name;
    rhi::Buffer* buffer = nullptr;
    uint64 offset = 0;
    uint64 size = 0;
    uint64 stride = 0;
};

struct DispatchRaysInfo
{
    ShaderIdentifierBuffer rayGeneration;
    ShaderIdentifierBuffer miss;
    ShaderIdentifierBuffer hitGroup;
    ShaderIdentifierBuffer callable;
    uint32 width = 1;
    uint32 height = 1;
    uint32 depth = 1;
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

using TextureHandle = Texture*;
using TextureViewHandle = TextureView*;
using BufferHandle = Buffer*;
using BufferViewHandle = BufferView*;

using ResourceVariant = std::variant<
    Buffer*,
    BufferView*,
    Texture*,
    TextureView*,
    Shader*,
    Sampler*,
    Pipeline*,
    CommandPool*,
    CommandBuffer*,
    SwapChain*,
    Fence*,
    Semaphore*,
    AccelerationStructure*
>;

constexpr uint32 g_queueCount = std::underlying_type_t<rhi::QueueType>(rhi::QueueType::COUNT);
}

ENABLE_BIT_MASK(fe::rhi::BLAS::Geometry::Flags)
ENABLE_BIT_MASK(fe::rhi::TLAS::Instance::Flags)
ENABLE_BIT_MASK(fe::rhi::AccelerationStructureInfo::Flags)
