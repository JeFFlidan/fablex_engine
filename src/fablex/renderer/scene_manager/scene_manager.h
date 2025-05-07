#pragma once

#include "gpu_model.h"
#include "gpu_texture.h"
#include "gpu_material.h"
#include "command_recorder.h"
#include "common.h"

#include "engine/entity/entity.h"
#include "engine/components/fwd.h"
#include "shaders/shader_interop_renderer.h"

#include <array>
#include <memory>

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

    bool is_texture_loaded_into_gpu(UUID textureUUID) const;
    void load_texture_into_gpu(UUID textureUUID) const;

    void add_staging_buffer(rhi::Buffer* buffer);
    const CommandRecorder& get_cmd_recorder(rhi::QueueType queueType) const;
    uint32 get_model_index(const GPUModel& gpuModel) const;
    uint32 get_instance_count(const GPUModel& gpuModel) const;
    int32 get_descriptor(asset::Texture* texture) const;
    int32 get_sampler_descriptor(ResourceName samplerName) const;
    const GPUTexture& get_blue_noise_texture() const;

    const std::vector<GPUModel>& get_gpu_models() const { return m_gpuModels; }
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

    struct GPUMaterialInfo
    {
        uint32 index = 0;
    };

    struct GPUTextureCreateInfo
    {
        asset::Texture* textureAsset;
        rhi::TextureInitInfo initInfo;
    };

    std::vector<CommandRecorderPtr> m_cmdRecorderPerQueue;

    EntityArray m_pendingEntities;
    
    std::vector<GPUTextureCreateInfo> m_pendingTextureCreateInfos;

    std::vector<engine::ModelComponent*> m_modelComponents;
    std::vector<engine::MaterialComponent*> m_materialComponents; 
    std::vector<engine::ShaderEntityComponent*> m_shaderEntityComponents;

    uint64 m_lightComponentCount = 0;
    uint64 m_lightEntityBufferOffset = 0;   // NOT IN BYTES!!!

    std::vector<DeleteHandlerArray> m_deleteHandlersPerFrame;

    std::unordered_map<ResourceName, rhi::Sampler*> m_samplerByName; 
    UUID m_blueNoiseTextureUUID = UUID::INVALID;

    std::vector<GPUModel> m_gpuModels;
    std::unordered_map<UUID, GPUModelInfo> m_gpuModelInfoByUUID;

    std::mutex m_textureMutex;
    std::unordered_map<UUID, std::unique_ptr<GPUTexture>> m_gpuTextureByUUID;

    std::vector<GPUMaterial> m_gpuMaterials;
    std::unordered_map<UUID, GPUMaterialInfo> m_gpuMaterialInfoByUUID;

    BufferArray m_modelBuffers;
    BufferArray m_modelInstanceBuffers;
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
    void init_callbacks();
    void load_resources();
    void create_samplers();

    void set_cmd(rhi::CommandBuffer* cmd);
    void allocate_storage_buffers();
    rhi::Buffer* get_model_buffer() const;
    rhi::Buffer* get_model_instance_buffer() const;
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