#pragma once

#include "globals.h"
#include "utils.h"
#include "rhi/rhi.h"
#include "core/types.h"
#include <vector>

namespace fe::renderer
{

template<typename DataType>
struct DefaultGPUDataBuffersAllocator
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

template<typename DataType, typename Allocator = DefaultGPUDataBuffersAllocator<DataType>>
class GPUDataBuffers
{
public:
    void set_debug_name(const char* debugName)
    {
        m_debugName = debugName;
    }

    void cleanup()
    {
        for (rhi::Buffer* buffer : m_buffers)
            rhi::destroy_buffer(buffer);
    }

    // Must be called before any invocation of increase_entry_count (in the beginning of the SceneManager update)
    void reset()
    {
        m_entryCount = 0;
    }

    // Must be called after entry count was fully updated using increase_entry_count function
    void allocate()
    {
        Allocator::allocate(m_buffers, m_entryCount, m_debugName);
    }

    void increase_entry_count(uint32 value)
    {
        m_entryCount += value;
    }

    void decrease_entry_count(uint32 value)
    {
        m_entryCount -= value;
    }

    DataType& operator[](uint32 index) const
    {
        rhi::Buffer* buffer = get_buffer();

        DataType* dataArray = static_cast<DataType*>(buffer->mappedData);
        FE_CHECK(buffer->size >= (index + 1) * sizeof(DataType));

        return dataArray[index];
    }

    uint32 descriptor() const
    {
        return get_buffer()->descriptorIndex;
    }

private:
    uint32 m_entryCount = 0;
    std::vector<rhi::Buffer*> m_buffers;
    const char* m_debugName = "BufferPlaceholderName";

    rhi::Buffer* get_buffer() const
    {
        return m_buffers.at(g_frameIndex);
    }
};

}
