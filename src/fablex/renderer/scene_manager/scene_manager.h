#pragma once

#include "common.h"
#include "tlas.h"
#include "frame_data.h"
#include "camera_data.h"
#include "gpu_data_buffers.h"
#include "gpu_resource_handle.h"
#include "gpu_resource_container.h"
#include "resource_destroyer.h"
#include "command_recorder_manager.h"

#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

class CommandRecorder;

using ModelBuffers = GPUDataStorageBuffers<ShaderModel>;
using ModelInstanceBuffers = GPUDataStorageBuffers<ShaderModelInstance>;
using MeshInstanceBuffers = GPUDataStorageBuffers<ShaderMeshInstance>;
using MaterialBuffers = GPUDataStorageBuffers<ShaderMaterial>;
using ShaderEntityBuffers = GPUDataStorageBuffers<ShaderEntity>;
using FrameDataBuffers = GPUDataUniformBuffers<FrameData>;
using CameraBuffers = GPUDataUniformBuffers<CameraData>;

struct SceneManagerCmds
{
    // Graphics cmd is main.
    rhi::CommandBuffer* graphicsCmd = nullptr;

    // Compute cmd is use to build tlases and blases
    rhi::CommandBuffer* computeCmd = nullptr;
};

class SceneManager
{
public:
    using ForEachModelHandler = std::function<void(const GPUModel& gpuModel, uint32 modelIndex)>;

    SceneManager();
    ~SceneManager();

    void upload(const SceneManagerCmds& cmds);

    void for_each_model(const ForEachModelHandler& handler);

    void add_staging_buffer(rhi::Buffer* buffer);
    void enqueue_destruction(const DestroyHandler& handler);
    int32 descriptor(asset::Texture* texture) const;
    int32 sampler_descriptor(ResourceName samplerName) const;
    const GPUTexture& blue_noise_texture() const;

    void record_graphics_cmd(const CommandRecorder::CmdRecordHandler& handler) const;
    void record_compute_cmd(const CommandRecorder::CmdRecordHandler& handler) const;

    const TLAS& tlas() const { return m_tlas; }
    uint32 tlas_descriptor() const { return m_tlas.descriptor(); }

    const ModelBuffers& model_buffers() const { return m_modelBuffers; }
    const ModelInstanceBuffers& model_instance_buffers() const { return m_modelInstanceBuffers; }
    const MeshInstanceBuffers& mesh_instance_buffers() const { return m_meshInstanceBuffers; }
    const MaterialBuffers& material_buffers() const { return m_materialBuffers; }
    const ShaderEntityBuffers& shader_entity_buffers() const { return m_shaderEntityBuffers; }

    GPUModel* gpu_model(UUID modelUUID) const;
    GPUTexture* gpu_texture(UUID textureUUID) const;
    GPUMaterial* gpu_material(UUID materialUUID) const;

private:
    ModelBuffers m_modelBuffers;
    ModelInstanceBuffers m_modelInstanceBuffers;
    MeshInstanceBuffers m_meshInstanceBuffers;
    MaterialBuffers m_materialBuffers;
    ShaderEntityBuffers m_shaderEntityBuffers;
    FrameDataBuffers m_frameDataBuffers;
    CameraBuffers m_cameraBuffers;

    GPUResourceContainer m_gpuResources;

    TLAS m_tlas;

    std::unordered_map<ResourceName, rhi::Sampler*> m_samplerByName; 
    UUID m_blueNoiseTextureUUID = UUID::INVALID;

    ResourceDestroyer m_resourceDestroyer;
    CommandRecorderManager m_commandRecorderManager;

    void subscribe_to_events();
    void load_resources();
    void create_samplers();

    void allocate_buffers();

    void upload_frame_data_to_gpu();
    void upload_cameras_to_gpu();
};

}