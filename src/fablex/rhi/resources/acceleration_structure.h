#pragma once

#include "internal.h"

namespace fe::rhi
{

class Buffer;

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

struct TLASInstance
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

struct TLAS
{

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

}

ENABLE_BIT_MASK(fe::rhi::BLAS::Geometry::Flags)
ENABLE_BIT_MASK(fe::rhi::TLASInstance::Flags)
ENABLE_BIT_MASK(fe::rhi::AccelerationStructureInfo::Flags)
