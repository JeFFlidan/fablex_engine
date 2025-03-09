#pragma once

#include "types.h"
#include "memory_utils.h"

namespace fe
{

template<typename T, typename ...Params>
T* memory_new(Params... params)
{
    return new(MemoryUtils::allocate_aligned_memory(sizeof(T), alignof(T))) T(params...);
}

inline void* memory_new(uint64 size, uint64 alignment)
{
    return MemoryUtils::allocate_aligned_memory(size, alignment);
}

template<typename T>
void memory_delete(T*& ptr)
{
    if (!ptr)
        return;

    ptr->~T();
    MemoryUtils::free_aligned_memory(ptr);
    ptr = nullptr;
}

}