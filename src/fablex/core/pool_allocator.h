#pragma once

#include "memory_utils.h"
#include "types.h"
#include "macro.h"
#include <vector>
#include <mutex>

namespace fe
{

template<typename T, size_t PoolSize = 64>
class PoolAllocator
{
public:
    PoolAllocator()
    {
        m_freePointers.reserve(PoolSize);
        m_memoryBlobs.reserve(4);
    }

    virtual ~PoolAllocator()
    {
        m_freePointers.clear();

        for (T* memoryBlob : m_memoryBlobs)
        {
            MemoryUtils::free_aligned_memory(memoryBlob);
        }

        m_memoryBlobs.clear();
    }

    template<typename... Params>
    T* allocate(Params&&... params)
    {
        T* objPtr;

        if (m_freePointers.empty())
        {
            if (m_nextIndex >= PoolSize || m_memoryBlobs.empty())
            {
                allocate_memory_blob();
                m_nextIndex = 0;
            }

            objPtr = &m_memoryBlobs.back()[m_nextIndex++];
        }
        else
        {
            objPtr = m_freePointers.back();
            m_freePointers.pop_back();
        }

        new(objPtr) T(std::forward<Params>(params)...);
        ++m_allocatedObjectCount;
        return objPtr;
    }

    void free(T* ptr)
    {
        ptr->~T();
        m_freePointers.push_back(ptr);
        --m_allocatedObjectCount;
    }

private:
    std::vector<T*> m_freePointers;
    std::vector<T*> m_memoryBlobs;
    uint32 m_nextIndex = 0;
    uint32 m_allocatedObjectCount = 0;

    void allocate_memory_blob()
    {
        T* memoryBlob = static_cast<T*>(MemoryUtils::allocate_aligned_memory(PoolSize * sizeof(T), alignof(T)));

        if (!memoryBlob)
        {
            FE_CHECK(0);
        }

        m_memoryBlobs.push_back(memoryBlob);
    }
};

template<typename T, size_t PoolSize = 64>
class ThreadSafePoolAllocator : private PoolAllocator<T, PoolSize>
{
public:
    ThreadSafePoolAllocator() : PoolAllocator<T, PoolSize>() { }

    template<typename... Params>
    T* allocate(Params&&... params)
    {
        std::scoped_lock<std::mutex> locker{m_threadLock};
        return PoolAllocator<T, PoolSize>::allocate(std::forward<Params>(params)...);
    }

    void free(T* ptr)
    {
        std::scoped_lock<std::mutex> locker{m_threadLock};
        return PoolAllocator<T, PoolSize>::free(ptr);
    }

private:
    std::mutex m_threadLock;
};

}
