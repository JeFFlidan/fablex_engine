#pragma once

#include "common.h"
#include "command_recorder.h"
#include "gpu_data_buffers.h"
#include "gpu_resource_handle.h"
#include "gpu_resource_container.h"
#include "resource_destroyer.h"

#include "core/fwd.h"
#include "engine/entity/entity.h"
#include "engine/components/fwd.h"
#include "shaders/shader_interop_renderer.h"

#include <array>
#include <memory>

namespace fe::renderer
{

class CommandRecorder;

using ModelBuffers = GPUDataBuffers<ShaderModel>;
using ModelInstanceBuffers = GPUDataBuffers<ShaderModelInstance>;
using MeshInstanceBuffers = GPUDataBuffers<ShaderMeshInstance>;
using MaterialBuffers = GPUDataBuffers<ShaderMaterial>;
using ShaderEntityBuffers = GPUDataBuffers<ShaderEntity>;

class SceneManager
{
public:
    using ForEachModelHandler = std::function<void(const GPUModel& gpuModel, uint32 modelIndex)>;

    SceneManager();
    ~SceneManager();

    void upload(rhi::CommandBuffer* cmd);
    void build_bvh(rhi::CommandBuffer* cmd);

    void for_each_model(const ForEachModelHandler& handler);

    void add_staging_buffer(rhi::Buffer* buffer);
    const CommandRecorder& cmd_recorder(rhi::QueueType queueType) const;
    int32 descriptor(asset::Texture* texture) const;
    int32 sampler_descriptor(ResourceName samplerName) const;
    const GPUTexture& blue_noise_texture() const;

    rhi::AccelerationStructure* scene_tlas() const { return m_TLAS; }

    const ModelBuffers& model_buffers() const { return m_modelBuffers; }
    const ModelInstanceBuffers& model_instance_buffers() const { return m_modelInstanceBuffers; }
    const MeshInstanceBuffers& mesh_instance_buffers() const { return m_meshInstanceBuffers; }
    const MaterialBuffers& material_buffers() const { return m_materialBuffers; }
    const ShaderEntityBuffers& shader_entity_buffers() const { return m_shaderEntityBuffers; }

    GPUModel* gpu_model(UUID modelUUID) const;
    GPUTexture* gpu_texture(UUID textureUUID) const;
    GPUMaterial* gpu_material(UUID materialUUID) const;

private:
    using ShaderCameraArray = std::array<ShaderCamera, MAX_CAMERA_COUNT>;
    using BufferArray = std::vector<rhi::Buffer*>;
    using EntityArray = std::vector<engine::Entity*>;
    using CommandRecorderPtr = std::unique_ptr<CommandRecorder>;
    using GPUResourceIndex = uint32;

    using GPUResourceHandleMap = std::unordered_map<UUID, GPUResourceHandlePtr>;

    ModelBuffers m_modelBuffers;
    ModelInstanceBuffers m_modelInstanceBuffers;
    MeshInstanceBuffers m_meshInstanceBuffers;
    MaterialBuffers m_materialBuffers;
    ShaderEntityBuffers m_shaderEntityBuffers;

    GPUResourceContainer m_gpuResources;

    std::vector<GPUModel*> m_gpuModels;

    uint32 m_modelCount = 0;
    uint32 m_modelInstanceCount = 0;
    uint32 m_meshInstanceCount = 0;
    uint32 m_materialCount = 0;
    
    uint64 m_lightComponentCount = 0;
    uint64 m_lightEntityBufferOffset = 0;   // NOT IN BYTES!!!

    std::vector<CommandRecorderPtr> m_cmdRecorderPerQueue;
    
    std::mutex m_gpuPendingTexturesMutex;

    std::unordered_map<ResourceName, rhi::Sampler*> m_samplerByName; 
    UUID m_blueNoiseTextureUUID = UUID::INVALID;

    ResourceDestroyer m_resourceDestroyer;

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

    void reset_per_frame_buffers();

    void set_cmd(rhi::CommandBuffer* cmd);
    void allocate_storage_buffers();

    void fill_frame_data();
    void fill_camera_buffers();

    void fill_tlas(rhi::CommandBuffer* cmd);
};

}