#pragma once

#include "gpu_model.h"
#include "command_recorder.h"
#include "engine/entity/entity.h"
#include "asset_manager/model/model.h"
#include "core/pool_allocator.h"
#include "shaders/shader_interop_renderer.h"
#include <array>

namespace fe::renderer
{

constexpr uint64 ONE_MEBIBYTE = 1048576;
constexpr uint64 DEFAULT_GPU_BUFFER_SIZE = ONE_MEBIBYTE * 32;

class CommandRecorder;

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    void upload(rhi::CommandBuffer* cmd);
    void build_bvh(rhi::CommandBuffer* cmd);

    void add_staging_buffer(rhi::Buffer* buffer);
    const CommandRecorder& get_cmd_recorder(rhi::QueueType queueType) const;
    uint32 get_model_index(GPUModel* gpuModel) const;
    uint32 get_instance_count(GPUModel* gpuModel) const;

    const std::vector<GPUModel*>& get_gpu_models() const { return m_gpuModels; }
    rhi::AccelerationStructure* get_scene_TLAS() const { return m_TLAS; }

private:
    using ShaderCameraArray = std::array<ShaderCamera, MAX_CAMERA_COUNT>;
    using BufferArray = std::vector<rhi::Buffer*>;
    using EntityArray = std::vector<engine::Entity*>;
    using DeleteHandler = std::function<void()>;
    using DeleteHandlerArray = std::vector<DeleteHandler>;
    using CommandRecorderPtr = std::unique_ptr<CommandRecorder>;

    struct GPUModelInfo
    {
        uint32 index = 0;
        uint32 instanceCount = 0;
    };

    std::vector<CommandRecorderPtr> m_cmdRecorderPerQueue;

    EntityArray m_entities;
    EntityArray m_pendingEntities;

    uint64 m_modelComponentCount = 0;

    std::vector<DeleteHandlerArray> m_deleteHandlersPerFrame;

    PoolAllocator<GPUModel, asset::AssetPoolSize<asset::Model>::poolSize> m_gpuModelAllocator;
    std::mutex m_gpuModelMutex;
    std::vector<GPUModel*> m_gpuModels;
    std::unordered_map<UUID, GPUModelInfo> m_gpuModelInfoByUUID;

    BufferArray m_modelBuffers;
    BufferArray m_modelInstanceBuffers;

    FrameUB m_frameData;
    ShaderCameraArray m_cameras;
    engine::Entity* m_mainCameraEntity = nullptr;
    BufferArray m_frameBuffers;
    BufferArray m_cameraBuffers;

    std::vector<engine::Entity*> m_entitiesForTLAS;
    rhi::AccelerationStructure* m_TLAS = nullptr;
    std::vector<rhi::Buffer*> m_uploadBuffersForTLAS;

    void set_cmd(rhi::CommandBuffer* cmd);
    rhi::Buffer* get_model_buffer();
    rhi::Buffer* get_model_instance_buffer();
    uint64 calc_buffer_size(uint64 currentSize, uint64 cpuEntrieSize);

    void fill_frame_data();
    void fill_camera_buffers();

    void add_delete_handler(const DeleteHandler& deleteHandler);

    rhi::Buffer* create_uma_storage_buffer(uint64 size = DEFAULT_GPU_BUFFER_SIZE) const;
    rhi::Buffer* create_uma_uniform_buffer(uint64 size) const;

    void fill_tlas(rhi::CommandBuffer* cmd);
};

}