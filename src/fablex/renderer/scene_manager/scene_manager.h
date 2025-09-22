#pragma once

#include "common.h"
#include "command_recorder.h"
#include "gpu_data_buffers.h"
#include "gpu_resource_handle.h"

#include "core/fwd.h"
#include "engine/entity/entity.h"
#include "engine/components/fwd.h"
#include "shaders/shader_interop_renderer.h"

#include <array>
#include <memory>
#include <functional>

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
    uint32 resource_index(UUID resourceUUID) const;
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
    using DeleteHandler = std::function<void()>;
    using DeleteHandlerArray = std::vector<DeleteHandler>;
    using CommandRecorderPtr = std::unique_ptr<CommandRecorder>;
    using GPUResourceIndex = uint32;

    using GPUResourceHandlePtr = std::unique_ptr<GPUResourceHandle>;
    using GPUResourceHandleArray = std::vector<GPUResourceHandlePtr>;
    using GPUResourceHandleArrayIterator = GPUResourceHandleArray::iterator;

    ModelBuffers m_modelBuffers;
    ModelInstanceBuffers m_modelInstanceBuffers;
    MeshInstanceBuffers m_meshInstanceBuffers;
    MaterialBuffers m_materialBuffers;
    ShaderEntityBuffers m_shaderEntityBuffers;

    GPUResourceHandleArray m_gpuResources;
    std::unordered_map<UUID, GPUResourceIndex> m_gpuResourcesLookup;

    std::vector<GPUModel*> m_gpuModels;

    uint32 m_modelCount = 0;
    uint32 m_modelInstanceCount = 0;
    uint32 m_meshInstanceCount = 0;
    uint32 m_materialCount = 0;
    
    uint64 m_lightComponentCount = 0;
    uint64 m_lightEntityBufferOffset = 0;   // NOT IN BYTES!!!

    std::vector<CommandRecorderPtr> m_cmdRecorderPerQueue;
    
    std::mutex m_gpuPendingTexturesMutex;

    std::vector<DeleteHandlerArray> m_deleteHandlersPerFrame;

    std::unordered_map<ResourceName, rhi::Sampler*> m_samplerByName; 
    UUID m_blueNoiseTextureUUID = UUID::INVALID;

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

    GPUModel* add_gpu_model(asset::Model* model, TaskGroup& taskGroup);
    GPUTexture* add_gpu_texutre(asset::Texture* texture);
    GPUMaterial* add_gpu_material(UUID materialUUID, TaskGroup& taskGroup);

    GPUResourceHandle* get_gpu_resource_handle(uint32 index) const;

    void set_cmd(rhi::CommandBuffer* cmd);
    void allocate_storage_buffers();

    void fill_frame_data();
    void fill_camera_buffers();

    void add_delete_handler(const DeleteHandler& deleteHandler);

    void fill_tlas(rhi::CommandBuffer* cmd);

    template<typename T>
    T* get_gpu_resource(UUID uuid) const
    {
        auto it = m_gpuResourcesLookup.find(uuid);
        if (it != m_gpuResourcesLookup.end())
            return get_gpu_resource_handle(it->second)->get<T>();

        return nullptr;
    }

    template<typename GPUResourceHandleType, typename... Params>
    GPUResourceHandleType::Type* add_gpu_resource(UUID uuid, Params&&... params)
    {
        using Type = GPUResourceHandleType::Type;

        m_gpuResourcesLookup[uuid] = m_gpuResources.size();
        return m_gpuResources.emplace_back(new GPUResourceHandleType(std::forward<Params>(params)...))->template get<Type>();
    }
};

}