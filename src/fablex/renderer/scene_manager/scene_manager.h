#pragma once

#include "gpu_model.h"
#include "engine/entity/entity.h"
#include "asset_manager/model/model.h"
#include "core/pool_allocator.h"

namespace fe::renderer
{

constexpr uint64 ONE_MEBIBYTE = 1048576;
constexpr uint64 DEFAULT_GPU_BUFFER_SIZE = ONE_MEBIBYTE * 32;

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    void upload(rhi::CommandBuffer* cmd);

private:
    std::vector<engine::Entity*> m_entities;
    std::vector<engine::Entity*> m_pendingEntities;

    uint64 m_modelComponentCount = 0;

    PoolAllocator<GPUModel, asset::AssetPoolSize<asset::Model>::poolSize> m_gpuModelAllocator;
    std::vector<GPUModel*> m_gpuModels;
    std::unordered_map<UUID, uint32> m_gpuModelIndexByUUID;

    std::vector<rhi::Buffer*> m_modelBuffers;
    std::vector<rhi::Buffer*> m_modelInstanceBuffers;

    rhi::Buffer* get_model_buffer();
    rhi::Buffer* get_model_instance_buffer();
    uint64 calc_buffer_size(uint64 currentSize, uint64 cpuEntrieSize);

    rhi::Buffer* create_uma_buffer(uint64 size = DEFAULT_GPU_BUFFER_SIZE) const;
};

}