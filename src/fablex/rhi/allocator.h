#pragma once

#include "resources.h"
#include "core/memory/pool_allocator.h"

namespace fe::rhi
{

template<typename ResourceType>
struct ResourcePoolSize {};

#define FE_SET_RHI_RESOURCE_POOL_SIZE(ResourceType, PoolSize)   \
    template<>                                                  \
    struct ResourcePoolSize<ResourceType>                       \
    {                                                           \
        static constexpr size_t Size = PoolSize;                \
    };

FE_SET_RHI_RESOURCE_POOL_SIZE(SwapChain, 4);
FE_SET_RHI_RESOURCE_POOL_SIZE(Pipeline, 1024);
FE_SET_RHI_RESOURCE_POOL_SIZE(Shader, 1024);
FE_SET_RHI_RESOURCE_POOL_SIZE(Buffer, 512);
FE_SET_RHI_RESOURCE_POOL_SIZE(Texture, 256);
FE_SET_RHI_RESOURCE_POOL_SIZE(TextureView, 512);
FE_SET_RHI_RESOURCE_POOL_SIZE(BufferView, 512);
FE_SET_RHI_RESOURCE_POOL_SIZE(Sampler, 256);
FE_SET_RHI_RESOURCE_POOL_SIZE(CommandPool, 64);
FE_SET_RHI_RESOURCE_POOL_SIZE(CommandBuffer, 256);
FE_SET_RHI_RESOURCE_POOL_SIZE(Semaphore, 24);
FE_SET_RHI_RESOURCE_POOL_SIZE(Fence, 8);
FE_SET_RHI_RESOURCE_POOL_SIZE(AccelerationStructure, 512);

template<typename ResourceType>
class ResourceAllocator
{
public:
    template<typename... Params>
    static ResourceType* allocate(Params&&... params)
    {
        return s_allocator.allocate(std::forward<Params>(params)...);
    }

    static void free(ResourceType* ptr)
    {
        s_allocator.free(ptr);
    }

private:
    inline static ThreadSafePoolAllocator<ResourceType> s_allocator;
};

}