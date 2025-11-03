#pragma once

#include "globals.h"
#include "utils.h"
#include "handles/buffer.h"
#include "handles/handle_vector.h"
#include "rhi/rhi.h"
#include "rhi/resources/buffer.h"
#include "core/types.h"
#include <vector>

namespace fe::renderer
{

template<typename DataType>
struct DefaultGPUDataStorageBuffersAllocator
{
    using TypedBufferHandle = TBufferHandle<DataType>;
    using TypedBufferRef = TBufferRef<DataType>;

    static uint32 get_new_buffer_size(uint32 currentSize, uint32 entryCount)
    {
        uint32 cpuEntriesSize = sizeof(DataType) * entryCount;
        
        if (currentSize * 2 < cpuEntriesSize)
            return cpuEntriesSize * 2;

        return currentSize * 2;
    }

    static void allocate(HandleVector<TypedBufferHandle>& inOutBuffers, uint32 entryCount, const char* debugName)
    {
        uint64 cpuEntriesSize = entryCount * sizeof(DataType);

        if (inOutBuffers.size() < g_frameIndex + 1)
        {
            uint64 bufferSize = cpuEntriesSize > DEFAULT_GPU_BUFFER_SIZE 
                ? get_new_buffer_size(0, cpuEntriesSize) : DEFAULT_GPU_BUFFER_SIZE;

            inOutBuffers.emplace(
                BufferCreateInfo
                {
                    .size = bufferSize,
                    .bufferUsage = ResourceUsage::STORAGE_BUFFER,
                    .memoryUsage = MemoryUsage::CPU_TO_GPU,
                }
            );

            Utils::set_debug_name(inOutBuffers.back(), debugName);
        }
        else
        {
            TypedBufferHandle& buffer = inOutBuffers.at(g_frameIndex);
            if (buffer->size < cpuEntriesSize)
            {
                uint64 currentBufferSize = buffer->size;
                uint64 newSize = get_new_buffer_size(currentBufferSize, cpuEntriesSize);

                buffer.init(
                    BufferCreateInfo
                    {
                        .size = newSize,
                        .bufferUsage = ResourceUsage::STORAGE_BUFFER,
                        .memoryUsage = MemoryUsage::CPU_TO_GPU,
                    }
                );

                Utils::set_debug_name(inOutBuffers.back(), debugName);
            }
        }
    }
};

template<typename DataType, uint32 Count = 1>
struct DefaultGPUDataUniformBuffersAllocator
{
    using TypedBufferHandle = TBufferHandle<DataType>;
    using TypedBufferRef = TBufferRef<DataType>;

    static void allocate(HandleVector<TypedBufferHandle>& inOutBuffers, uint32 entryCount, const char* debugName)
    {
        constexpr uint32 bufferSize = sizeof(DataType) * Count;

        if (inOutBuffers.size() < g_frameIndex + 1)
        {
            inOutBuffers.emplace(
                BufferCreateInfo
                {
                    .size = bufferSize,
                    .bufferUsage = ResourceUsage::UNIFORM_BUFFER,
                    .memoryUsage = MemoryUsage::CPU_TO_GPU,
                });

            Utils::set_debug_name(inOutBuffers.back(), debugName);
        }
    }
};

template<typename DataType, typename Allocator>
class GPUDataBuffers
{
public:
    using TypedBufferHandle = TBufferHandle<DataType>;
    using TypedBufferRef = TBufferRef<DataType>;

    void set_debug_name(const char* debugName)
    {
        m_debugName = debugName;
    }

    void allocate(uint32 entryCount)
    {
        Allocator::allocate(m_buffers, entryCount, m_debugName);
    }

    DataType& operator[](uint32 index) const
    {
        return *active_buffer_typed()[index];
    }

    // In bytes
    uint32 size() const
    {
        return active_buffer()->size;
    }

    uint8* data() const
    {
        return active_buffer().mapped_data();
    }

    uint32 descriptor() const
    {
        return active_buffer()->descriptorIndex;
    }

    BufferRef active_buffer() const
    {
        return m_buffers.at(g_frameIndex);
    }

    TypedBufferRef active_buffer_typed() const
    {
        return m_buffers.at(g_frameIndex);
    }

    void memset(int32 value)
    {
        std::memset(data(), value, size());
    }

    void bind_uniform_buffer(uint32 slot) const
    {
        TypedBufferRef buffer = active_buffer();

        if (buffer.has_usage(ResourceUsage::UNIFORM_BUFFER))
            buffer.bind_uniform_buffer(slot);
    }

private:
    HandleVector<TypedBufferHandle> m_buffers;
    const char* m_debugName = "BufferPlaceholderName";
};

template<typename T>
using GPUDataStorageBuffers = GPUDataBuffers<T, DefaultGPUDataStorageBuffersAllocator<T>>;

template<typename T>
using GPUDataUniformBuffers = GPUDataBuffers<T, DefaultGPUDataUniformBuffersAllocator<T>>;

}
