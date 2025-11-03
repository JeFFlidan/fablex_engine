#include "scene_manager.h"
#include "gpu_resource_counters.h"
#include "renderer/globals.h"
#include "renderer/device.h"

#include "rhi/rhi.h"
#include "rhi/resources/sampler.h"

#include "engine/entity/entity.h"
#include "engine/components/events.h"
#include "engine/components/model_component.h"
#include "engine/components/light_components.h"
#include "engine/components/camera_component.h"

#include "core/task_composer.h"
#include "asset_manager/asset_manager.h"
#include "asset_manager/events.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

constexpr const char* MODEL_BUFFER_NAME = "ModelBuffer";
constexpr const char* MODEL_INSTANCE_BUFFER_NAME = "ModelInstanceBuffer";
constexpr const char* MESH_INSTANCE_BUFFER_NAME = "MeshInstanceBuffer";
constexpr const char* ENTITY_BUFFER_NAME = "EntityBuffer";
constexpr const char* MATERIAL_BUFFER_NAME = "MaterialBuffer";
constexpr const char* FRAME_DATA_BUFFER_NAME = "FrameDataBuffer";
constexpr const char* CAMERA_BUFFER_NAME = "CameraBuffer";

SceneManager::SceneManager(DeletionQueue* deletionQueue) 
    : m_gpuResources(this), m_tlas(this), m_deletionQueue(deletionQueue)
{
    // For now some buffers support only CPU_TO_GPU memory usage
    FE_CHECK(Device::has_capability(GPUCapability::CACHE_COHERENT_UMA));
    FE_CHECK(Device::has_capability(GPUCapability::RAY_TRACING));

    subscribe_to_events();
    // load_resources();
    create_samplers();

    m_modelBuffers.set_debug_name(MODEL_BUFFER_NAME);
    m_modelInstanceBuffers.set_debug_name(MODEL_INSTANCE_BUFFER_NAME);
    m_meshInstanceBuffers.set_debug_name(MESH_INSTANCE_BUFFER_NAME);
    m_shaderEntityBuffers.set_debug_name(ENTITY_BUFFER_NAME);
    m_materialBuffers.set_debug_name(MATERIAL_BUFFER_NAME);
    m_cameraBuffers.set_debug_name(CAMERA_BUFFER_NAME);
    m_frameDataBuffers.set_debug_name(FRAME_DATA_BUFFER_NAME);
}

SceneManager::~SceneManager()
{

}

void SceneManager::upload(const SceneManagerCmds& cmds)
{
    m_commandRecorderManager.set_cmd(cmds.graphicsCmd);
    m_commandRecorderManager.set_cmd(cmds.computeCmd);

    TaskGroup taskGroup;
    
    m_gpuResources.reset();
    GPUResourceCounters::reset();

    m_tlas.allocate(GPUResourceCounters::model_instance_count());

    engine::ModelComponent::for_each([this, &taskGroup](engine::ModelComponent* modelComponent)
    {
        GPUModel* gpuModel = m_gpuResources.add_model(modelComponent->get_model_uuid(), taskGroup);

        uint32 meshCount = modelComponent->get_model()->meshes().size();
        GPUResourceCounters::increase_mesh_instance_count(meshCount);

        gpuModel->increase_ref_count(1);;
    });

    engine::MaterialComponent::for_each([this, &taskGroup](engine::MaterialComponent* materialComponent)
    {
        std::vector<UUID> textureUUIDs;

        for (UUID materialUUID : materialComponent->material_uuids())
        {
            GPUMaterial* gpuMaterial = m_gpuResources.add_material(materialUUID, taskGroup);
            
            gpuMaterial->increase_ref_count(1);
            gpuMaterial->asset()->get_texture_uuids(textureUUIDs);

            for (UUID textureUUID : textureUUIDs)
            {
                GPUTexture* gpuTexture = m_gpuResources.add_texture(textureUUID, taskGroup);
                gpuTexture->increase_ref_count(1); 
            }

            textureUUIDs.clear();
        }
    });

    TaskComposer::wait(taskGroup);

    allocate_buffers();

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        m_gpuResources.process();
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        uint32 modelInstanceOffset = 0;
        uint32 meshInstanceOffset = 0;

        engine::ModelComponent::for_each([&](engine::ModelComponent* modelComponent)
        {
            GPUModel* gpuModel = m_gpuResources.model(modelComponent->get_model_uuid());
            FE_CHECK(gpuModel);

            gpuModel->update_instance_offsets(modelInstanceOffset, meshInstanceOffset);
            gpuModel->upload_model_instance(this, modelComponent->get_entity());
        });

        m_tlas.build();
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        engine::LightComponent::for_each([this](engine::LightComponent* lightComponent)
        {
            ShaderEntity& shaderEntity = m_shaderEntityBuffers[GPUResourceCounters::next_light_index()];
            shaderEntity.init();
            lightComponent->fill_shader_data(shaderEntity);
        });
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        upload_frame_data_to_gpu();
        upload_cameras_to_gpu();
    });

    TaskComposer::wait(taskGroup);
}

void SceneManager::for_each_model(const ForEachModelHandler& handler)
{
    engine::ModelComponent::for_each([this, handler](engine::ModelComponent* modelComponent)
    {
        GPUModel* gpuModel = m_gpuResources.model(modelComponent->get_model_uuid());
        handler(*gpuModel, gpuModel->index());
    });
}

int32 SceneManager::descriptor(asset::Texture* texture) const
{
    if (GPUTexture* gpuTexture = gpu_texture(texture->get_uuid()))
        return gpuTexture->texture_view()->descriptorIndex;
    return -1;
}

int32 SceneManager::sampler_descriptor(ResourceName samplerName) const
{
    auto it = m_samplerByName.find(samplerName);
    if (it == m_samplerByName.end())
        return -1;
    return it->second->descriptorIndex;
}

const GPUTexture& SceneManager::blue_noise_texture() const
{
    GPUTexture* gpuTexture = gpu_texture(m_blueNoiseTextureUUID);
    FE_CHECK(gpuTexture);
    return *gpuTexture;
}

void SceneManager::record_graphics_cmd(const CommandRecorder::CmdRecordHandler& handler) const
{
    m_commandRecorderManager.record_graphics_cmd(handler);
}

void SceneManager::record_compute_cmd(const CommandRecorder::CmdRecordHandler& handler) const
{
    m_commandRecorderManager.record_compute_cmd(handler);
}

void SceneManager::add_staging_buffer(rhi::Buffer* buffer)
{
    FE_CHECK(buffer);
    
    m_deletionQueue->add([buffer]()
    {
        rhi::destroy_buffer(buffer);
    });
}

void SceneManager::enqueue_destruction(DeletionHandler&& handler)
{
    m_deletionQueue->add(std::move(handler));
}

void SceneManager::subscribe_to_events()
{
    // Temp solution for built in resources
    EventManager::subscribe<asset::TextureImportedEvent>([this](const auto& event)
    {
        asset::Texture* texture = event.get_handle();
        if (texture->has_flag(asset::AssetFlag::TRANSIENT))
        {
            TaskGroup taskGroup;
            m_gpuResources.add_texture(texture->get_uuid(), taskGroup);
        }
    });
}

// This function is not used now because I don't need builtin textures.
// Have to think how to handle builtin resource storing when open new project.
void SceneManager::load_resources()
{
    asset::TextureImportContext blueNoiseImportContext;
    blueNoiseImportContext.originalFilePath = "content/BlueNoise3DIndependent.dds";
    blueNoiseImportContext.projectDirectory = " ";
    blueNoiseImportContext.flags = asset::AssetFlag::TRANSIENT;

    asset::TextureImportResult blueNoiseImportResult;

    asset::AssetManager::import_texture(blueNoiseImportContext, blueNoiseImportResult);
    m_blueNoiseTextureUUID = blueNoiseImportResult.texture->get_uuid();
}

void SceneManager::create_samplers()
{
    SamplerCreateInfo samplerInfo;
    samplerInfo.filter = rhi::Filter::MIN_MAG_MIP_LINEAR;
    samplerInfo.addressMode = rhi::AddressMode::REPEAT;
    samplerInfo.borderColor = rhi::BorderColor::FLOAT_TRANSPARENT_BLACK;
    samplerInfo.maxAnisotropy = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = std::numeric_limits<float>::max();
    m_samplerByName[SAMPLER_LINEAR_REPEAT] = SamplerHandle(samplerInfo);

    samplerInfo.addressMode = rhi::AddressMode::CLAMP_TO_EDGE;
    m_samplerByName[SAMPLER_LINEAR_CLAMP] = SamplerHandle(samplerInfo);

    samplerInfo.addressMode = rhi::AddressMode::MIRRORED_REPEAT;
    m_samplerByName[SAMPLER_LINEAR_MIRROR] = SamplerHandle(samplerInfo);

    samplerInfo.filter = rhi::Filter::MIN_MAG_MIP_NEAREST;
    samplerInfo.addressMode = rhi::AddressMode::REPEAT;
    m_samplerByName[SAMPLER_NEAREST_REPEAT] = SamplerHandle(samplerInfo);

    samplerInfo.addressMode = rhi::AddressMode::CLAMP_TO_EDGE;
    m_samplerByName[SAMPLER_NEAREST_CLAMP] = SamplerHandle(samplerInfo);

    samplerInfo.addressMode = rhi::AddressMode::MIRRORED_REPEAT;
    m_samplerByName[SAMPLER_NEAREST_MIRROR] = SamplerHandle(samplerInfo);

    samplerInfo.addressMode = rhi::AddressMode::CLAMP_TO_EDGE;
    samplerInfo.filter = rhi::Filter::MINIMUM_MIN_MAG_LINEAR_MIP_NEAREST;
    m_samplerByName[SAMPLER_MINIMUM_NEAREST_CLAMP] = SamplerHandle(samplerInfo);
}

GPUModel* SceneManager::gpu_model(UUID modelUUID) const
{
    return m_gpuResources.model(modelUUID);
}

GPUTexture* SceneManager::gpu_texture(UUID textureUUID) const
{
    return m_gpuResources.texture(textureUUID);
}

GPUMaterial* SceneManager::gpu_material(UUID materialUUID) const
{
    return m_gpuResources.material(materialUUID);
}

void SceneManager::allocate_buffers()
{
    m_modelBuffers.allocate(GPUResourceCounters::model_count());
    m_modelInstanceBuffers.allocate(GPUResourceCounters::model_instance_count());
    m_meshInstanceBuffers.allocate(GPUResourceCounters::mesh_instance_count());
    m_materialBuffers.allocate(GPUResourceCounters::material_count());
    m_shaderEntityBuffers.allocate(GPUResourceCounters::shader_entity_count());
    m_cameraBuffers.allocate(0);
    m_frameDataBuffers.allocate(0);
}

void SceneManager::upload_frame_data_to_gpu()
{
    FrameData& frameData = m_frameDataBuffers[0];
    frameData.fill(this);

    m_frameDataBuffers.bind_uniform_buffer(UB_FRAME_SLOT);
}

void SceneManager::upload_cameras_to_gpu()
{
    uint32 index = 0;

    engine::CameraComponent::for_each([this, &index](engine::CameraComponent* cameraComponent)
    {
        CameraData& cameraData = m_cameraBuffers[index++];
        cameraData.fill(cameraComponent);
    });

    m_cameraBuffers.bind_uniform_buffer(UB_CAMERA_SLOT);
}

}