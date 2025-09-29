#pragma once

#include "core/memory/pool_allocator.h"

namespace fe::renderer
{

template<typename ResourceType>
class GPUResourceAllocator
{
public:
    template<typename... Params>
    static ResourceType* allocate(Params&&... params)
    {
        return s_allocator.allocate(std::forward<Params>(params)...);
    }

    static void free(ResourceType* resource)
    {
        s_allocator.free(resource);
    }

private:
    inline static PoolAllocator<ResourceType, 1024> s_allocator;
};

}