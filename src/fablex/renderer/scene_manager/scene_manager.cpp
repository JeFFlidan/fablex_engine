#include "scene_manager.h"
#include "renderer/globals.h"
#include "renderer/utils.h"

#include "rhi/rhi.h"
#include "rhi/utils.h"
#include "engine/entity/events.h"
#include "engine/components/events.h"
#include "engine/components/model_component.h"
#include "engine/components/editor_camera_component.h"
#include "engine/components/light_components.h"

#include "core/utils.h"
#include "core/primitives/sphere.h"
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

SceneManager::SceneManager() : m_gpuResources(this)
{
    // For now some buffers support only CPU_TO_GPU memory usage
    FE_CHECK(rhi::has_capability(rhi::GPUCapability::CACHE_COHERENT_UMA));
    FE_CHECK(rhi::has_capability(rhi::GPUCapability::RAY_TRACING));

    allocate_arrays();
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
    m_modelBuffers.cleanup();
    m_modelInstanceBuffers.cleanup();
    m_meshInstanceBuffers.cleanup();
    m_materialBuffers.cleanup();
    m_shaderEntityBuffers.cleanup();
    m_frameDataBuffers.cleanup();
    m_cameraBuffers.cleanup();

    for (rhi::Buffer* buffer : m_uploadBuffersForTLAS)
        rhi::destroy_buffer(buffer);

    for (auto [name, sampler] : m_samplerByName)
        rhi::destroy_sampler(sampler);

    m_gpuModels.clear();

    m_resourceDestroyer.process_all();

    if (m_TLAS)
    {
        rhi::destroy_buffer(m_TLAS->info.tlas.instanceBuffer);
        rhi::destroy_acceleration_structure(m_TLAS);
    }
}

void SceneManager::upload(rhi::CommandBuffer* cmd)
{
    set_cmd(cmd);

    m_resourceDestroyer.process_current_frame();

    reset_per_frame_buffers();

    TaskGroup taskGroup;

    m_modelCount = 0;
    m_materialCount = 0;
    
    m_lightEntityBufferOffset = 0;
    m_lightComponentCount = engine::LightComponent::count();

    m_shaderEntityBuffers.increase_entry_count(engine::ShaderEntityComponent::count());

    m_gpuResources.reset();

    m_gpuModels.clear();
    m_entitiesForTLAS.clear();

    engine::ModelComponent::for_each([this, &taskGroup](engine::ModelComponent* modelComponent)
    {
        GPUModel* gpuModel = m_gpuResources.add_model(modelComponent->get_model_uuid(), taskGroup);

        uint32 meshCount = modelComponent->get_model()->meshes().size();

        if (gpuModel->ref_count() == 0)
        {
            gpuModel->index(m_gpuModels.size());
            
            m_modelBuffers.increase_entry_count(1);
            m_gpuModels.push_back(gpuModel);
        }
        
        gpuModel->increase_ref_count(1);
        m_modelInstanceBuffers.increase_entry_count(1);
        m_meshInstanceBuffers.increase_entry_count(meshCount);
    });

    engine::MaterialComponent::for_each([this, &taskGroup](engine::MaterialComponent* materialComponent)
    {
        for (UUID materialUUID : materialComponent->material_uuids())
        {
            GPUMaterial* gpuMaterial = m_gpuResources.add_material(materialUUID, taskGroup);

            if (gpuMaterial->ref_count() == 0)
                gpuMaterial->index(m_materialCount++);

            gpuMaterial->increase_ref_count(1);
        }

        m_materialBuffers.increase_entry_count(materialComponent->material_uuids().size());
    });

    TaskComposer::wait(taskGroup);

    allocate_buffers();

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        m_gpuResources.process();
    });

    m_entitiesForTLAS.resize(engine::ModelComponent::count());
    
    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        uint32 modelInstanceOffset = 0;
        uint32 meshInstanceOffset = 0;

        engine::ModelComponent::for_each([&](engine::ModelComponent* modelComponent)
        {
            GPUModel* gpuModel = m_gpuResources.model(modelComponent->get_model_uuid());
            FE_CHECK(gpuModel);

            gpuModel->update_instance_offsets(modelInstanceOffset, meshInstanceOffset);
            m_entitiesForTLAS[gpuModel->next_model_instance_index()] = modelComponent->get_entity();
            gpuModel->upload_model_instance(this, modelComponent->get_entity());
        });
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        uint32 index = 0;

        engine::LightComponent::for_each([this, &index](engine::LightComponent* lightComponent)
        {
            ShaderEntity& shaderEntity = m_shaderEntityBuffers[m_lightEntityBufferOffset + index++];
            shaderEntity.init();
            lightComponent->fill_shader_data(shaderEntity);
        });
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        FE_LOG(LogDefault, INFO, "BEFORE DATA FRAME UB");
        upload_frame_data_to_gpu();
        FE_LOG(LogDefault, INFO, "BEFORE CAMERAS FRAME UB");
        upload_cameras_to_gpu();
        FE_LOG(LogDefault, INFO, "AFTER ALL UB");
    });

    TaskComposer::wait(taskGroup);
}

void SceneManager::build_bvh(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd->cmdPool->queueType == rhi::QueueType::COMPUTE);

    set_cmd(cmd);

    for (GPUModel* gpuModel : m_gpuModels)
        gpuModel->build_blas(cmd_recorder(rhi::QueueType::COMPUTE));

    fill_tlas(cmd);
}

void SceneManager::for_each_model(const ForEachModelHandler& handler)
{
    for (const GPUModel* gpuModel : m_gpuModels)
        handler(*gpuModel, gpuModel->index());
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

void SceneManager::add_staging_buffer(rhi::Buffer* buffer)
{
    FE_CHECK(buffer);
    
    m_resourceDestroyer.enqueue_destruction([buffer]()
    {
        rhi::destroy_buffer(buffer);
    });
}

const CommandRecorder& SceneManager::cmd_recorder(rhi::QueueType queueType) const
{
    return *m_cmdRecorderPerQueue.at(rhi::get_queue_index(queueType));
}

void SceneManager::allocate_arrays()
{
    m_gpuModels.reserve(asset::AssetPoolSize<asset::Model>::poolSize);

    for (uint32 i = 0; i != rhi::g_queueCount; ++i)
        m_cmdRecorderPerQueue.emplace_back(new CommandRecorder());
}

void SceneManager::subscribe_to_events()
{
    EventManager::subscribe<asset::TextureLoadedEvent>([this](const auto& event)
    {
        TaskGroup taskGroup;    // TEMP
        m_gpuResources.add_texture(event.get_handle()->get_uuid(), taskGroup);
    });

    EventManager::subscribe<asset::TextureImportedEvent>([this](const auto& event)
    {
        TaskGroup taskGroup;    // TEMP
        m_gpuResources.add_texture(event.get_handle()->get_uuid(), taskGroup);
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
    auto createSampler = [&](Name name, const rhi::SamplerInfo& info)
    {
        if (m_samplerByName.contains(name))
        {
            FE_LOG(LogRenderer, ERROR, "Sampler {} already exists.", name.to_string());
            return;
        }
    
        rhi::Sampler* sampler = nullptr;
        rhi::create_sampler(&sampler, &info);
        m_samplerByName[name] = sampler;
    };

    rhi::SamplerInfo samplerInfo;
    samplerInfo.filter = rhi::Filter::MIN_MAG_MIP_LINEAR;
    samplerInfo.addressMode = rhi::AddressMode::REPEAT;
    samplerInfo.borderColor = rhi::BorderColor::FLOAT_TRANSPARENT_BLACK;
    samplerInfo.maxAnisotropy = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = std::numeric_limits<float>::max();
    createSampler(SAMPLER_LINEAR_REPEAT, samplerInfo);

    samplerInfo.addressMode = rhi::AddressMode::CLAMP_TO_EDGE;
    createSampler(SAMPLER_LINEAR_CLAMP, samplerInfo);

    samplerInfo.addressMode = rhi::AddressMode::MIRRORED_REPEAT;
    createSampler(SAMPLER_LINEAR_MIRROR, samplerInfo);

    samplerInfo.filter = rhi::Filter::MIN_MAG_MIP_NEAREST;
    samplerInfo.addressMode = rhi::AddressMode::REPEAT;
    createSampler(SAMPLER_NEAREST_REPEAT, samplerInfo);

    samplerInfo.addressMode = rhi::AddressMode::CLAMP_TO_EDGE;
    createSampler(SAMPLER_NEAREST_CLAMP, samplerInfo);

    samplerInfo.addressMode = rhi::AddressMode::MIRRORED_REPEAT;
    createSampler(SAMPLER_NEAREST_MIRROR, samplerInfo);

    samplerInfo.addressMode = rhi::AddressMode::CLAMP_TO_EDGE;
    samplerInfo.filter = rhi::Filter::MINIMUM_MIN_MAG_LINEAR_MIP_NEAREST;
    createSampler(SAMPLER_MINIMUM_NEAREST_CLAMP, samplerInfo);
}

void SceneManager::reset_per_frame_buffers()
{
    m_modelBuffers.reset();
    m_modelInstanceBuffers.reset();
    m_meshInstanceBuffers.reset();
    m_materialBuffers.reset();
    m_shaderEntityBuffers.reset();
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

void SceneManager::set_cmd(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd);
    
    cmd_recorder(cmd->cmdPool->queueType).set_cmd(cmd);
}

void SceneManager::allocate_buffers()
{
    m_modelBuffers.allocate();
    m_modelInstanceBuffers.allocate();
    m_meshInstanceBuffers.allocate();
    m_materialBuffers.allocate();
    m_shaderEntityBuffers.allocate();
    m_cameraBuffers.allocate();
    m_frameDataBuffers.allocate();
}

void SceneManager::upload_frame_data_to_gpu()
{
    FrameUB& frameData = m_frameDataBuffers[0];
    frameData.modelBufferIndex = m_modelBuffers.descriptor();
    frameData.modelInstanceBufferIndex = m_modelInstanceBuffers.descriptor();
    frameData.meshInstanceBufferIndex = m_meshInstanceBuffers.descriptor();
    frameData.entityBufferIndex = m_shaderEntityBuffers.descriptor();
    frameData.materialBufferIndex = m_materialBuffers.descriptor();
    frameData.lightArrayCount = m_lightComponentCount;
    frameData.lightArrayOffset = m_lightEntityBufferOffset;

    m_frameDataBuffers.bind_uniform_buffer(UB_FRAME_SLOT);
}

void SceneManager::upload_cameras_to_gpu()
{
    uint32 index = 0;

    engine::CameraComponent::for_each([this, &index](engine::CameraComponent* cameraComponent)
    {
        ShaderCamera& shaderCamera = m_cameraBuffers[index++];

        shaderCamera.position = cameraComponent->get_entity()->get_position();
        shaderCamera.view = cameraComponent->view;
        shaderCamera.projection = cameraComponent->projection;
        shaderCamera.viewProjection = cameraComponent->viewProjection;
        shaderCamera.prevViewProjection = cameraComponent->prevViewProjection;
        shaderCamera.inverseView = cameraComponent->inverseView;
        shaderCamera.inverseProjection = cameraComponent->inverseProjection;
        shaderCamera.inverseViewProjection = cameraComponent->inverseViewProjection;
        shaderCamera.zNear = cameraComponent->zNear;
        shaderCamera.zFar = cameraComponent->zFar;
        shaderCamera.create_frustum();
    });

    m_cameraBuffers.bind_uniform_buffer(UB_CAMERA_SLOT);
}

void SceneManager::fill_tlas(rhi::CommandBuffer* cmd)
{
    uint64 instanceSize = rhi::get_acceleration_structure_instance_size();
    uint64 objectCount = m_entitiesForTLAS.size() + 1 * 2;

    if (!m_TLAS || m_TLAS->info.tlas.count < objectCount)
    {
        if (m_TLAS)
        {
            rhi::AccelerationStructure* oldTLAS = m_TLAS;
            m_resourceDestroyer.enqueue_destruction([oldTLAS]()
            {
                rhi::Buffer* instanceBuffer = oldTLAS->info.tlas.instanceBuffer;
                rhi::destroy_acceleration_structure(oldTLAS);
                rhi::destroy_buffer(instanceBuffer);
            });
        }
    
        rhi::AccelerationStructureInfo info;
        info.flags = rhi::AccelerationStructureInfo::Flags::PREFER_FAST_BUILD;
        info.type = rhi::AccelerationStructureInfo::TOP_LEVEL;
        info.tlas.count = objectCount;
    
        rhi::BufferInfo bufferInfo;
        bufferInfo.bufferUsage = 
            rhi::ResourceUsage::STORAGE_BUFFER |
            rhi::ResourceUsage::TRANSFER_DST;
        bufferInfo.memoryUsage = rhi::MemoryUsage::GPU;
        bufferInfo.size = info.tlas.count * instanceSize;
        bufferInfo.flags = rhi::ResourceFlags::RAY_TRACING;
    
        rhi::create_buffer(&info.tlas.instanceBuffer, &bufferInfo);
        rhi::create_acceleration_structure(&m_TLAS, &info);

        rhi::set_name(m_TLAS->info.tlas.instanceBuffer, "TLASInstanceBuffer");
        rhi::set_name(m_TLAS, "MainTLAS");
    }

    if (m_uploadBuffersForTLAS.size() < g_frameIndex + 1
        || m_uploadBuffersForTLAS.at(g_frameIndex)->size / instanceSize < objectCount
    )
    {
        if (m_uploadBuffersForTLAS.size() < g_frameIndex + 1)
            m_uploadBuffersForTLAS.push_back(nullptr);

        if (m_uploadBuffersForTLAS.at(g_frameIndex))
            rhi::destroy_buffer(m_uploadBuffersForTLAS.at(g_frameIndex));

        rhi::BufferInfo bufferInfo;
        bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC;
        bufferInfo.memoryUsage = rhi::MemoryUsage::CPU;
        bufferInfo.size = m_TLAS->info.tlas.count * instanceSize;
        rhi::create_buffer(&m_uploadBuffersForTLAS.at(g_frameIndex), &bufferInfo);
    }

    rhi::Buffer* uploadBuffer = m_uploadBuffersForTLAS.at(g_frameIndex);
    uint8* instanceBufferPtr = (uint8*)uploadBuffer->mappedData;
    memset(instanceBufferPtr, 0, uploadBuffer->size);

    uint32 instanceCount = 0;
    for (engine::Entity* entity : m_entitiesForTLAS)
    {
        if (engine::ModelComponent* modelComponent = entity->get_component<engine::ModelComponent>())
        {
            GPUModel* gpuModel = gpu_model(modelComponent->get_model_uuid());
            FE_CHECK(gpuModel);

            rhi::TLAS::Instance instance;
            instance.instanceID = instanceCount;

            instance.blas = gpuModel->blases().at(0);
            instance.instanceMask = 1 << 0; // TEMP
            instance.instanceContributionToHitGroupIndex = 0;
            instance.flags = rhi::TLAS::Instance::Flags::TRIANGLE_CULL_DISABLE;

            Matrix remapMat = modelComponent->get_model()->aabb().get_unorm_remap_matrix();
            Float4x4 transformMat = remapMat * entity->get_world_transform();

            for (uint32 i = 0; i != ARRAYSIZE(instance.transform); ++i)
                for (uint32 j = 0; j != ARRAYSIZE(instance.transform[i]); ++j)
                    instance.transform[i][j] = transformMat.m[j][i];

            void* dst = instanceBufferPtr + instanceCount * instanceSize;

            rhi::write_top_level_acceleration_structure_instance(&instance, dst);

            ++instanceCount;
        }
    }

    rhi::Buffer* instanceBuffer = m_TLAS->info.tlas.instanceBuffer;
    rhi::copy_buffer(cmd, uploadBuffer, instanceBuffer, uploadBuffer->size, 0, 0);

    rhi::PipelineBarrier barrier(
        instanceBuffer,
        rhi::ResourceLayout::TRANSFER_DST,
        rhi::ResourceLayout::SHADER_READ
    );

    rhi::build_acceleration_structure(cmd, m_TLAS, nullptr);
}

}