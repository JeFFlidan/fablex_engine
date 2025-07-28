#pragma once

namespace fe
{

class MemoryUtils
{
public:
    static void* allocate_aligned_memory(size_t size, size_t alignment);
    static void free_aligned_memory(void* ptr);
};

}