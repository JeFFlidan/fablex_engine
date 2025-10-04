#pragma once

#include "gpu_data_buffers.h"
#include "engine/entity/fwd.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/acceleration_structure.h"

namespace fe::renderer
{

class GPUModel;
class SceneManager;

struct TLASUploadBuffersAllocator
{
    static void allocate(std::vector<rhi::Buffer*>& inOutBuffers, uint32 entryCount, const char* debugName)
    {
        uint64 instanceSize = rhi::get_acceleration_structure_instance_size();

        if (inOutBuffers.size() < g_frameIndex + 1
            || inOutBuffers.at(g_frameIndex)->size / instanceSize < entryCount
        )
        {
            if (inOutBuffers.size() < g_frameIndex + 1)
                inOutBuffers.emplace_back();

            if (inOutBuffers.at(g_frameIndex))
                rhi::destroy_buffer(inOutBuffers.at(g_frameIndex));

            rhi::BufferInfo bufferInfo;
            bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC;
            bufferInfo.memoryUsage = rhi::MemoryUsage::CPU;
            bufferInfo.size = entryCount * instanceSize;
            rhi::create_buffer(&inOutBuffers.at(g_frameIndex), &bufferInfo);
            Utils::set_debug_name(inOutBuffers.at(g_frameIndex), debugName);
        }
    };
};

// rhi::TLASInstance is used here as a placeholder, these buffers should not be indexed to get the typed structure, only raw bytes should be used.
using TLASUploadBuffers = GPUDataBuffers<rhi::TLASInstance, TLASUploadBuffersAllocator>;

class TLAS
{
public:
    TLAS(SceneManager* sceneManager);
    ~TLAS();

    void allocate(uint32 inObjectCount);
    void write(const GPUModel* model, const engine::Entity* entity, uint32 instanceIndex) const;
    void build();

    uint32 descriptor() const
    {
        return m_tlas->descriptorIndex;
    }

protected:
    TLASUploadBuffers m_uploadBuffers;
    rhi::AccelerationStructure* m_tlas = nullptr;
    SceneManager* m_sceneManager = nullptr;

    uint64 instance_size() const;
    rhi::Buffer* instance_buffer() const;
};

}