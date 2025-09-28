#pragma once

#include "globals.h"
#include "utils.h"
#include "rhi/rhi.h"
#include "core/types.h"
#include <vector>

namespace fe::renderer
{

template<typename DataType>
struct DefaultGPUDataStorageBuffersAllocator
{
    static uint32 get_new_buffer_size(uint32 currentSize, uint32 entryCount)
    {
        uint32 cpuEntriesSize = sizeof(DataType) * entryCount;
        
        if (currentSize * 2 < cpuEntriesSize)
            return cpuEntriesSize * 2;

        return currentSize * 2;
    }

    static void allocate(std::vector<rhi::Buffer*>& inOutBuffers, uint32 entryCount, const char* debugName)
    {
        uint64 cpuEntriesSize = entryCount * sizeof(DataType);

        if (inOutBuffers.size() < g_frameIndex + 1)
        {
            uint64 bufferSize = cpuEntriesSize > DEFAULT_GPU_BUFFER_SIZE 
                ? get_new_buffer_size(0, cpuEntriesSize) : DEFAULT_GPU_BUFFER_SIZE;

            inOutBuffers.push_back(Utils::create_uma_storage_buffer(bufferSize));
            Utils::set_debug_name(inOutBuffers.back(), debugName);
        }
        else
        {
            rhi::Buffer* buffer = inOutBuffers.at(g_frameIndex);
            if (buffer->size < cpuEntriesSize)
            {
                uint64 currentBufferSize = buffer->size;
                rhi::destroy_buffer(buffer);

                uint64 newSize = get_new_buffer_size(currentBufferSize, cpuEntriesSize);
                inOutBuffers.at(g_frameIndex) = Utils::create_uma_storage_buffer(newSize);
                Utils::set_debug_name(inOutBuffers.back(), debugName);
            }
        }
    }
};

template<typename DataType, uint32 Count = 1>
struct DefaultGPUDataUniformBuffersAllocator
{
    static void allocate(std::vector<rhi::Buffer*>& inOutBuffers, uint32 entryCount, const char* debugName)
    {
        constexpr uint32 bufferSize = sizeof(DataType) * Count;

        if (inOutBuffers.size() < g_frameIndex + 1)
        {
            inOutBuffers.push_back(Utils::create_uma_uniform_buffer(bufferSize));
            Utils::set_debug_name(inOutBuffers.back(), debugName);
        }
    }
};

template<typename DataType, typename Allocator>
class GPUDataBuffers
{
public:
    ~GPUDataBuffers()
    {
        cleanup();
    }

    void set_debug_name(const char* debugName)
    {
        m_debugName = debugName;
    }

    void cleanup()
    {
        for (rhi::Buffer* buffer : m_buffers)
            rhi::destroy_buffer(buffer);
    }

    void allocate(uint32 entryCount)
    {
        Allocator::allocate(m_buffers, entryCount, m_debugName);
    }

    DataType& operator[](uint32 index) const
    {
        rhi::Buffer* buffer = active_buffer();

        DataType* dataArray = static_cast<DataType*>(buffer->mappedData);
        FE_CHECK(buffer->size >= (index + 1) * sizeof(DataType));

        return dataArray[index];
    }

    // In bytes
    uint32 size() const
    {
        return active_buffer()->size;
    }

    uint8* data() const
    {
        return static_cast<uint8*>(active_buffer()->mappedData);
    }

    uint32 descriptor() const
    {
        return active_buffer()->descriptorIndex;
    }

    rhi::Buffer* active_buffer() const
    {
        return m_buffers.at(g_frameIndex);
    }

    void memset(int32 value)
    {
        std::memset(data(), value, size());
    }

    void bind_uniform_buffer(uint32 slot) const
    {
        rhi::Buffer* buffer = active_buffer();

        if (has_flag(buffer->bufferUsage, rhi::ResourceUsage::UNIFORM_BUFFER))
            rhi::bind_uniform_buffer(buffer, g_frameIndex, slot, buffer->size, 0);
    }

private:
    std::vector<rhi::Buffer*> m_buffers;
    const char* m_debugName = "BufferPlaceholderName";
};

template<typename T>
using GPUDataStorageBuffers = GPUDataBuffers<T, DefaultGPUDataStorageBuffersAllocator<T>>;

template<typename T>
using GPUDataUniformBuffers = GPUDataBuffers<T, DefaultGPUDataUniformBuffersAllocator<T>>;

}
