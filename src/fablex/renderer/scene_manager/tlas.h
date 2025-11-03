#pragma once

#include "gpu_data_buffers.h"
#include "handles/acceleration_structure.h"
#include "engine/entity/fwd.h"

namespace fe::renderer
{

class GPUModel;
class SceneManager;

struct TLASUploadBuffersAllocator
{
    using TLASInstanceBufferHandle = TBufferHandle<TLASInstance>;
    using BufferVector = HandleVector<TLASInstanceBufferHandle>;

    static void allocate(BufferVector& inOutBuffers, uint32 entryCount, const char* debugName);
};

// rhi::TLASInstance is used here as a placeholder, these buffers should not be indexed to get the typed structure, only raw bytes should be used.
using TLASUploadBuffers = GPUDataBuffers<TLASInstance, TLASUploadBuffersAllocator>;

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
        return m_tlas.descriptor();
    }

protected:
    TLASUploadBuffers m_uploadBuffers;
    TLASHandle m_tlas;
    BufferHandle m_instanceBuffer;
    SceneManager* m_sceneManager = nullptr;

    uint64 instance_size() const;
};

}