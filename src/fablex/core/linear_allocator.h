#pragma once

#include "types.h"
#include "memory_utils.h"
#include "macro.h"

namespace fe
{

class LinearAllocator
{
public:
    ~LinearAllocator()
    {
        MemoryUtils::free_aligned_memory(m_buffer);
    }

    constexpr uint64 get_capacity() const
    {
        return m_capacity;
    }

    void reserve(uint64 newCapacity, uint64 alignment)
    {
        m_alignment = alignment;
        newCapacity = align(newCapacity, m_alignment);
        m_capacity = newCapacity;

        MemoryUtils::free_aligned_memory(m_buffer);
        m_buffer = (uint8*)MemoryUtils::allocate_aligned_memory(m_capacity, m_alignment);
    }

    uint8* allocate(uint64 size)
    {
        size = align(size, m_alignment);
        if (m_offset + size <= m_capacity)
        {
            uint8* ret = &m_buffer[m_offset];
            m_offset += size;
            return ret;
        }
        return nullptr;
    }

    void free(uint64 size)
    {
        size = align(size, m_alignment);
        FE_CHECK(m_offset >= size);
        m_offset -= size;
    }

    void reset()
    {
        m_offset = 0;
    }

    uint8* top()
    {
        return &m_buffer[m_offset];
    }

private:
    uint8* m_buffer = nullptr;
    uint64 m_capacity = 0;
    uint64 m_offset = 0;
    uint64 m_alignment = 1;

    constexpr uint64 align(uint64 value, uint64 alignment)
    {
        return ((value + alignment - 1) / alignment) * alignment;
    }
};

}