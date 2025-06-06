#pragma once

#include "gpu_model.h"
#include "gpu_texture.h"
#include "gpu_material.h"
#include "command_recorder.h"
#include "common.h"

#include "core/fwd.h"
#include "engine/entity/entity.h"
#include "engine/components/fwd.h"
#include "shaders/shader_interop_renderer.h"

#include <array>
#include <memory>
#include <functional>

namespace fe::renderer
{

constexpr uint64 ONE_MEBIBYTE = 1048576;
constexpr uint64 DEFAULT_GPU_BUFFER_SIZE = ONE_MEBIBYTE * 32;

class CommandRecorder;

class SceneManager
{
public:
    using ForEachModelHandler = std::function<void(const GPUModel& gpuModel, uint32 modelIndex)>;

    SceneManager();
    ~SceneManager();

    void upload(rhi::CommandBuffer* cmd);
    void build_bvh(rhi::CommandBuffer* cmd);

    bool is_texture_loaded_into_gpu(UUID textureUUID) const;
    void load_texture_into_gpu(UUID textureUUID) const;

    void for_each_model(const ForEachModelHandler& handler);

    bool has_gpu_resource(UUID resourceUUID) const;

    void add_staging_buffer(rhi::Buffer* buffer);
    const CommandRecorder& cmd_recorder(rhi::QueueType queueType) const;
    uint32 resource_index(UUID resourceUUID) const;
    int32 descriptor(asset::Texture* texture) const;
    int32 sampler_descriptor(ResourceName samplerName) const;
    const GPUTexture& blue_noise_texture() const;

    rhi::AccelerationStructure* scene_tlas() const { return m_TLAS; }

private:
    using ShaderCameraArray = std::array<ShaderCamera, MAX_CAMERA_COUNT>;
    using BufferArray = std::vector<rhi::Buffer*>;
    using EntityArray = std::vector<engine::Entity*>;
    using DeleteHandler = std::function<void()>;
    using DeleteHandlerArray = std::vector<DeleteHandler>;
    using CommandRecorderPtr = std::unique_ptr<CommandRecorder>;
    using GPUResourceIndex = uint32;
    using GPUModelHandle = std::unique_ptr<GPUModel>;
    using GPUTextureHandle = std::unique_ptr<GPUTexture>;
    using GPUMaterialHandle = std::unique_ptr<GPUMaterial>;

    struct GPUPendingTexture
    {
        std::unique_ptr<GPUTexture> gpuTexture;
    };

    std::vector<CommandRecorderPtr> m_cmdRecorderPerQueue;

    std::unordered_set<engine::Entity*> m_pendingEntities;
    std::unordered_set<engine::MaterialComponent*> m_pendingMaterialComponents;
    std::vector<engine::ModelComponent*> m_pendingModelComponents;
    
    std::mutex m_gpuPendingTexturesMutex;
    std::vector<GPUPendingTexture> m_pendingTextures;
    std::vector<asset::Model*> m_pendingModels; 
    std::vector<asset::Material*> m_pendingMaterials;

    std::vector<engine::ModelComponent*> m_modelComponents;
    std::vector<engine::ShaderEntityComponent*> m_shaderEntityComponents;

    uint64 m_lightComponentCount = 0;
    uint64 m_lightEntityBufferOffset = 0;   // NOT IN BYTES!!!

    std::vector<DeleteHandlerArray> m_deleteHandlersPerFrame;

    std::unordered_map<ResourceName, rhi::Sampler*> m_samplerByName; 
    UUID m_blueNoiseTextureUUID = UUID::INVALID;

    std::unordered_map<UUID, GPUResourceIndex> m_gpuResourcesLookup;
    
    std::vector<GPUModelHandle> m_gpuModels;
    std::vector<GPUTextureHandle> m_gpuTextures;
    std::vector<GPUMaterialHandle> m_gpuMaterials;

    uint64 m_modelInstanceCount = 0;
    uint64 m_meshCount = 0;

    BufferArray m_modelBuffers;
    BufferArray m_modelInstanceBuffers;
    BufferArray m_meshInstanceBuffers;
    BufferArray m_materialBuffers;
    BufferArray m_shaderEntityBuffers;

    FrameUB m_frameData;
    ShaderCameraArray m_cameras;
    engine::Entity* m_mainCameraEntity = nullptr;
    BufferArray m_frameBuffers;
    BufferArray m_cameraBuffers;

    EntityArray m_entitiesForTLAS;
    rhi::AccelerationStructure* m_TLAS = nullptr;
    BufferArray m_uploadBuffersForTLAS;

    void allocate_arrays();
    void subscribe_to_events();
    void load_resources();
    void create_samplers();

    void add_gpu_model(asset::Model* model, TaskGroup& taskGroup);
    void add_gpu_material(UUID materialUUID, TaskGroup& taskGroup);
    void add_gpu_materials(const std::vector<UUID>& materialUUIDs, TaskGroup& taskGroup);

    GPUModel* get_gpu_model(UUID modelUUID) const;
    GPUTexture* get_gpu_texture(UUID textureUUID) const;
    GPUTexture* get_gpu_pending_texture(UUID textureUUID) const;
    GPUMaterial* get_gpu_material(UUID materialUUID) const;

    void set_cmd(rhi::CommandBuffer* cmd);
    void allocate_storage_buffers();
    rhi::Buffer* get_model_buffer() const;
    rhi::Buffer* get_model_instance_buffer() const;
    rhi::Buffer* get_mesh_instance_buffer() const;
    rhi::Buffer* get_material_buffer() const;
    rhi::Buffer* get_shader_entity_buffer() const;
    uint64 calc_buffer_size(uint64 currentSize, uint64 cpuEntrieSize);

    void fill_frame_data();
    void fill_camera_buffers();

    void add_delete_handler(const DeleteHandler& deleteHandler);

    rhi::Buffer* create_uma_storage_buffer(uint64 size = DEFAULT_GPU_BUFFER_SIZE) const;
    rhi::Buffer* create_uma_uniform_buffer(uint64 size) const;

    void fill_tlas(rhi::CommandBuffer* cmd);

    std::string generate_resource_name(const std::string& baseName) const;
};

}