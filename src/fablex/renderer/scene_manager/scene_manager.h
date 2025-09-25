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

using ModelBuffers = GPUDataStorageBuffers<ShaderModel>;
using ModelInstanceBuffers = GPUDataStorageBuffers<ShaderModelInstance>;
using MeshInstanceBuffers = GPUDataStorageBuffers<ShaderMeshInstance>;
using MaterialBuffers = GPUDataStorageBuffers<ShaderMaterial>;
using ShaderEntityBuffers = GPUDataStorageBuffers<ShaderEntity>;
using FrameDataBuffers = GPUDataUniformBuffers<FrameUB>;
using CameraBuffers = GPUDataUniformBuffers<ShaderCamera>;

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
    using BufferArray = std::vector<rhi::Buffer*>;
    using EntityArray = std::vector<engine::Entity*>;
    using CommandRecorderPtr = std::unique_ptr<CommandRecorder>;

    ModelBuffers m_modelBuffers;
    ModelInstanceBuffers m_modelInstanceBuffers;
    MeshInstanceBuffers m_meshInstanceBuffers;
    MaterialBuffers m_materialBuffers;
    ShaderEntityBuffers m_shaderEntityBuffers;
    FrameDataBuffers m_frameDataBuffers;
    CameraBuffers m_cameraBuffers;

    GPUResourceContainer m_gpuResources;

    std::vector<GPUModel*> m_gpuModels;

    uint32 m_modelCount = 0;
    uint32 m_materialCount = 0;
    
    uint64 m_lightComponentCount = 0;
    uint64 m_lightEntityBufferOffset = 0;   // NOT IN BYTES!!!

    std::vector<CommandRecorderPtr> m_cmdRecorderPerQueue;
    
    std::unordered_map<ResourceName, rhi::Sampler*> m_samplerByName; 
    UUID m_blueNoiseTextureUUID = UUID::INVALID;

    ResourceDestroyer m_resourceDestroyer;

    EntityArray m_entitiesForTLAS;
    rhi::AccelerationStructure* m_TLAS = nullptr;
    BufferArray m_uploadBuffersForTLAS;

    void allocate_arrays();
    void subscribe_to_events();
    void load_resources();
    void create_samplers();

    void reset_per_frame_buffers();

    void set_cmd(rhi::CommandBuffer* cmd);
    void allocate_buffers();

    void upload_frame_data_to_gpu();
    void upload_cameras_to_gpu();

    void fill_tlas(rhi::CommandBuffer* cmd);
};

}