#pragma once

#include "internal.h"

namespace fe::rhi
{

class Buffer;
class Texture;

class PipelineBarrier
{
public:
    enum BarrierType
    {
        MEMORY,
        BUFFER,
        TEXTURE
    } type;

    struct MemoryBarrier
    {
        ResourceLayout srcLayout;
        ResourceLayout dstLayout;
    };

    struct BufferBarrier
    {
        const Buffer* buffer;
        ResourceLayout srcLayout;
        ResourceLayout dstLayout;
    };

    struct TextureBarrier
    {
        const Texture* texture;
        ResourceLayout srcLayout;
        ResourceLayout dstLayout;
        uint32_t levelCount;
        uint32_t baseMipLevel;
        uint32_t layerCount;
        uint32_t baseLayer;
    };

    PipelineBarrier() = default;
    PipelineBarrier(const MemoryBarrier& memoryBarrier) { m_barrier.memoryBarrier = memoryBarrier; }
    PipelineBarrier(const BufferBarrier& bufferBarrier) { m_barrier.bufferBarrier = bufferBarrier; }
    PipelineBarrier(const TextureBarrier& textureBarrier) { m_barrier.textureBarrier = textureBarrier; }

    PipelineBarrier(ResourceLayout srcLayout, ResourceLayout dstLayout)
    { 
        m_barrier.memoryBarrier = MemoryBarrier{srcLayout, dstLayout};
        type = MEMORY;
    }

    PipelineBarrier(Buffer* buffer, ResourceLayout srcLayout, ResourceLayout dstLayout)
    { 
        m_barrier.bufferBarrier = BufferBarrier{buffer, srcLayout, dstLayout};
        type = BUFFER;
    }

    PipelineBarrier(Texture* texture,
        ResourceLayout srcLayout,
        ResourceLayout dstLayout,
        uint32_t baseMipLevel = 0,
        uint32_t levelCount = 0,
        uint32_t baseLayer = 0,
        uint32_t layerCount = 0)
    { 
        m_barrier.textureBarrier = TextureBarrier{texture, srcLayout, dstLayout, levelCount, baseMipLevel, layerCount, baseLayer};
        type = TEXTURE;
    }

    void set_memory_barrier(ResourceLayout srcLayout, ResourceLayout dstLayout)
    {
        m_barrier.memoryBarrier = MemoryBarrier{srcLayout, dstLayout};
        type = MEMORY;
    }

    void set_buffer_barrier(Buffer* buffer, ResourceLayout srcLayout, ResourceLayout dstLayout)
    {
        m_barrier.bufferBarrier = BufferBarrier{buffer, srcLayout, dstLayout};
        type = BUFFER;
    }

    void set_texture_barrier(Texture* texture,
        ResourceLayout srcLayout,
        ResourceLayout dstLayout,
        uint32_t baseMipLevel = 0,
        uint32_t levelCount = 0,
        uint32_t baseLayer = 0,
        uint32_t layerCount = 0)
    {
        m_barrier.textureBarrier = TextureBarrier{texture, srcLayout, dstLayout, levelCount, baseMipLevel, layerCount, baseLayer};
        type = TEXTURE;
    }

    const MemoryBarrier* get_memory_barrier() const { return &m_barrier.memoryBarrier; }
    const BufferBarrier* get_buffer_barrier() const { return &m_barrier.bufferBarrier; }
    const TextureBarrier* get_texture_barrier() const { return &m_barrier.textureBarrier; }

private:
    union
    {
        MemoryBarrier memoryBarrier;
        BufferBarrier bufferBarrier;
        TextureBarrier textureBarrier;
    } m_barrier;
};

}