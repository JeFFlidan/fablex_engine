#include "scene_manager.h"
#include "renderer/globals.h"

#include "rhi/rhi.h"
#include "rhi/utils.h"
#include "engine/entity/events.h"
#include "engine/components/model_component.h"
#include "engine/components/editor_camera_component.h"
#include "engine/components/light_components.h"
#include "core/primitives/sphere.h"
#include "core/task_composer.h"
#include "asset_manager/asset_manager.h"
#include "asset_manager/events.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

constexpr uint64 SHADER_ENTITY_INIT_COUNT = 1024ULL;
constexpr uint64 MATERIAL_INIT_COUNT = 256ULL;
constexpr uint64 TEXTURE_INIT_COUNT = 256ULL;

const std::string MODEL_BUFFER_NAME = "ModelBuffer";
const std::string MODEL_INSTANCE_BUFFER_NAME = "ModelInstanceBuffer";
const std::string MESH_INSTANCE_BUFFER_NAME = "MeshInstanceBuffer";
const std::string ENTITY_BUFFER_NAME = "EntityBuffer";
const std::string MATERIAL_BUFFER_NAME = "MaterialBuffer";
const std::string FRAME_DATA_BUFFER_NAME = "FrameDataBuffer";
const std::string CAMERA_BUFFER_NAME = "CameraBuffer";

SceneManager::SceneManager()
{
    // For now some buffers support only CPU_TO_GPU memory usage
    FE_CHECK(rhi::has_capability(rhi::GPUCapability::CACHE_COHERENT_UMA));
    FE_CHECK(rhi::has_capability(rhi::GPUCapability::RAY_TRACING));

    allocate_arrays();
    init_callbacks();
    // load_resources();
    create_samplers();
}

SceneManager::~SceneManager()
{
    for (rhi::Buffer* buffer : m_modelBuffers)
        rhi::destroy_buffer(buffer);

    for (rhi::Buffer* buffer : m_modelInstanceBuffers)
        rhi::destroy_buffer(buffer);

    for (rhi::Buffer* buffer : m_meshInstanceBuffers)
        rhi::destroy_buffer(buffer);

    for (rhi::Buffer* buffer : m_shaderEntityBuffers)
        rhi::destroy_buffer(buffer);

    for (rhi::Buffer* buffer : m_cameraBuffers)
        rhi::destroy_buffer(buffer);

    for (rhi::Buffer* buffer : m_frameBuffers)
        rhi::destroy_buffer(buffer);

    for (rhi::Buffer* buffer : m_uploadBuffersForTLAS)
        rhi::destroy_buffer(buffer);

    for (rhi::Buffer* buffer : m_materialBuffers)
        rhi::destroy_buffer(buffer);

    for (auto [name, sampler] : m_samplerByName)
        rhi::destroy_sampler(sampler);

    m_gpuModels.clear();
    m_gpuTextures.clear();

    for (const DeleteHandlerArray& deleteHandlers : m_deleteHandlersPerFrame)
        for (const DeleteHandler& deleteHandler : deleteHandlers)
            deleteHandler();

    if (m_TLAS)
    {
        rhi::destroy_buffer(m_TLAS->info.tlas.instanceBuffer);
        rhi::destroy_acceleration_structure(m_TLAS);
    }
}

void SceneManager::upload(rhi::CommandBuffer* cmd)
{
    set_cmd(cmd);

    if (m_deleteHandlersPerFrame.size() < g_frameIndex + 1)
        m_deleteHandlersPerFrame.emplace_back();

    DeleteHandlerArray& deleteHandlers = m_deleteHandlersPerFrame[g_frameIndex];
    for (const DeleteHandler& deleteHandler : deleteHandlers)
        deleteHandler();
    deleteHandlers.clear();

    TaskGroup taskGroup;

    for (engine::Entity* entity : m_pendingEntities)
    {
        if (engine::ModelComponent* modelComponent = entity->get_component<engine::ModelComponent>())
        {
            ++m_modelInstanceCount;
            m_meshCount += modelComponent->get_model()->meshes().size();

            m_entitiesForTLAS.push_back(entity);
            
            UUID modelUUID = modelComponent->get_model_uuid();

            if (GPUModel* gpuModel = get_gpu_model(modelUUID))
            {
                gpuModel->add_instance(entity);
            }
            else
            {
                uint64 index = m_gpuModels.size();
                m_gpuModels.emplace_back(new GPUModel(modelComponent));
                gpuModel = m_gpuModels.back().get();
                gpuModel->add_instance(entity);

                m_gpuResourcesLookup[modelUUID] = index;

                TaskComposer::execute(taskGroup, [this, gpuModel](TaskExecutionInfo execInfo)
                {
                    gpuModel->build(this, cmd_recorder(rhi::QueueType::GRAPHICS));
                });
            }
        }

        if (engine::MaterialComponent* materialComponent = entity->get_component<engine::MaterialComponent>())
            add_materials(materialComponent->material_uuids(), taskGroup);

        if (engine::EditorCameraComponent* cameraComponent = entity->get_component<engine::EditorCameraComponent>())
        {
            m_mainCameraEntity = entity;
        }

        if (engine::LightComponent* lightComponent = entity->get_component<engine::LightComponent>())
        {
            ++m_lightComponentCount;
            m_shaderEntityComponents.push_back(lightComponent);
        }
    }

    for (engine::MaterialComponent* materialComponent : m_pendingMaterials)
        add_materials(materialComponent->material_uuids(), taskGroup);

    m_pendingEntities.clear();
    m_pendingMaterials.clear();

    TaskComposer::wait(taskGroup);

    allocate_storage_buffers();

    // More offsets will be added further when new ShaderEntities will be created
    m_lightEntityBufferOffset = 0;

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        for (auto& [texture] : m_pendingTextures)
        {
            texture->build(cmd_recorder(rhi::QueueType::GRAPHICS));
        }
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        rhi::Buffer* buffer = get_model_buffer();
        ShaderModel* shaderModels = static_cast<ShaderModel*>(buffer->mappedData);
        uint64 index = 0;

        for (const GPUModelHandle& gpuModel : m_gpuModels)
            gpuModel->fill_shader_model(shaderModels[index++]);
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        rhi::Buffer* modelInstanceBuffer = get_model_instance_buffer();
        rhi::Buffer* meshInstanceBuffer = get_mesh_instance_buffer();

        ShaderModelInstance* shaderModelInstances = static_cast<ShaderModelInstance*>(modelInstanceBuffer->mappedData);
        ShaderMeshInstance* shaderMeshInstances = static_cast<ShaderMeshInstance*>(meshInstanceBuffer->mappedData);

        uint64 modelInstanceOffset = 0;
        uint64 meshInstanceOffset = 0;

        for (const GPUModelHandle& gpuModel : m_gpuModels)
        {
            gpuModel->fill_shader_model_and_mesh_instances(
                this, 
                shaderModelInstances, 
                modelInstanceOffset,
                shaderMeshInstances,
                meshInstanceOffset
            );
        }
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        rhi::Buffer* buffer = get_shader_entity_buffer();
        ShaderEntity* shaderEntities = static_cast<ShaderEntity*>(buffer->mappedData);
        uint64 lightIndex = 0;

        for (engine::ShaderEntityComponent* shaderEntityComponent : m_shaderEntityComponents)
        {
            if (shaderEntityComponent->is_light_source())
            {
                ShaderEntity& shaderEntity = shaderEntities[m_lightEntityBufferOffset + lightIndex++];
                shaderEntity.init();
                shaderEntityComponent->fill_shader_data(shaderEntity);
            }
        }
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        rhi::Buffer* buffer = get_material_buffer();
        ShaderMaterial* shaderMaterials = static_cast<ShaderMaterial*>(buffer->mappedData);
        uint64 index = 0;
        
        for (const GPUMaterialHandle& material : m_gpuMaterials)
        {
            ShaderMaterial& shaderMaterial = shaderMaterials[index++];
            shaderMaterial.init();
            material->fill_shader_material(this, shaderMaterial);
        }
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        fill_frame_data();
        fill_camera_buffers();
    });

    TaskComposer::wait(taskGroup);

    for (auto& [texture] : m_pendingTextures)
    {
        UUID textureUUID = texture->texture_asset()->get_uuid();
        uint64 index = m_gpuTextures.size();
        m_gpuTextures.push_back(std::move(texture));
        
        m_gpuResourcesLookup[textureUUID] = index;
    }

    m_pendingTextures.clear();
}

void SceneManager::build_bvh(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd->cmdPool->queueType == rhi::QueueType::COMPUTE);

    set_cmd(cmd);

    for (GPUModelHandle& gpuModel : m_gpuModels)
        gpuModel->build_blas(cmd_recorder(rhi::QueueType::COMPUTE));

    fill_tlas(cmd);
}

bool SceneManager::is_texture_loaded_into_gpu(UUID textureUUID) const
{
    return false;
}

void SceneManager::load_texture_into_gpu(UUID textureUUID) const
{

}

void SceneManager::for_each_model(const ForEachModelHandler& handler)
{
    uint32 index = 0;
    for (const GPUModelHandle& gpuModel : m_gpuModels)
        handler(*gpuModel, index++);
}

bool SceneManager::has_gpu_resource(UUID resourceUUID) const
{
    if (m_gpuResourcesLookup.find(resourceUUID) != m_gpuResourcesLookup.end())
        return true;
    return false;
}

int32 SceneManager::descriptor(asset::Texture* texture) const
{
    if (GPUTexture* gpuTexture = get_gpu_texture(texture->get_uuid()))
        return gpuTexture->texture_view()->descriptorIndex;
    if (GPUTexture* gpuTexture = get_gpu_pending_texture(texture->get_uuid()))
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
    GPUTexture* gpuTexture = get_gpu_texture(m_blueNoiseTextureUUID);
    FE_CHECK(gpuTexture);
    return *gpuTexture;
}

void SceneManager::add_staging_buffer(rhi::Buffer* buffer)
{
    FE_CHECK(buffer);
    
    add_delete_handler([buffer]()
    {
        rhi::destroy_buffer(buffer);
    });
}

const CommandRecorder& SceneManager::cmd_recorder(rhi::QueueType queueType) const
{
    return *m_cmdRecorderPerQueue.at(rhi::get_queue_index(queueType));
}

uint32 SceneManager::resource_index(UUID resourceUUID) const
{
    auto it = m_gpuResourcesLookup.find(resourceUUID);
    if (it == m_gpuResourcesLookup.end())
        return -1;
    return it->second;
}

void SceneManager::allocate_arrays()
{
    m_gpuModels.reserve(asset::AssetPoolSize<asset::Model>::poolSize);
    m_shaderEntityComponents.reserve(SHADER_ENTITY_INIT_COUNT);
    m_gpuMaterials.reserve(MATERIAL_INIT_COUNT);
    m_gpuTextures.reserve(TEXTURE_INIT_COUNT);

    for (uint32 i = 0; i != rhi::g_queueCount; ++i)
        m_cmdRecorderPerQueue.emplace_back(new CommandRecorder());
}

void SceneManager::init_callbacks()
{
    EventManager::subscribe<engine::EntityCreatedEvent>([this](const engine::EntityCreatedEvent& event)
    {
        m_pendingEntities.insert(event.get_entity());
    });

    EventManager::subscribe<asset::AssetLoadedEvent<asset::Texture>>([this](const auto& event)
    {
        asset::Texture* textureAsset = event.get_handle();
        if (m_gpuResourcesLookup.contains(textureAsset->get_uuid()))
            return;

        GPUPendingTexture& createInfo = m_pendingTextures.emplace_back();
        createInfo.gpuTexture = std::make_unique<GPUTexture>(textureAsset);
        
        createInfo.gpuTexture->create();
    });

    EventManager::subscribe<asset::AssetImportedEvent<asset::Texture>>([this](const auto& event)
    {
        asset::Texture* textureAsset = event.get_handle();
        if (m_gpuResourcesLookup.contains(textureAsset->get_uuid()))
            return;

        GPUPendingTexture& createInfo = m_pendingTextures.emplace_back();
        createInfo.gpuTexture = std::make_unique<GPUTexture>(textureAsset);
        
        createInfo.gpuTexture->create();
    });

    EventManager::subscribe<engine::MaterialUpdatedEvent>([this](const auto& event)
    {
        m_pendingMaterials.insert(event.material_component());
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

void SceneManager::add_materials(const std::vector<UUID>& materialUUIDs, TaskGroup& taskGroup)
{
    for (UUID materialUUID : materialUUIDs)
    {
        if (m_gpuResourcesLookup.contains(materialUUID))
            continue;

        uint64 index = m_gpuMaterials.size();
        m_gpuMaterials.emplace_back(new GPUMaterial(asset::AssetManager::get_material(materialUUID)));
        GPUMaterial* gpuMaterial = m_gpuMaterials.back().get();

        m_gpuResourcesLookup[materialUUID] = index;

        TaskComposer::execute(taskGroup, [this, gpuMaterial](TaskExecutionInfo execInfo)
        {
            gpuMaterial->build(this, cmd_recorder(rhi::QueueType::GRAPHICS));
        });
    }
}

GPUModel* SceneManager::get_gpu_model(UUID modelUUID) const
{
    auto it = m_gpuResourcesLookup.find(modelUUID);
    if (it == m_gpuResourcesLookup.end())
        return nullptr;
    return m_gpuModels.at(it->second).get();
}

GPUTexture* SceneManager::get_gpu_texture(UUID textureUUID) const
{
    auto it = m_gpuResourcesLookup.find(textureUUID);
    if (it == m_gpuResourcesLookup.end())
        return nullptr;
    return m_gpuTextures.at(it->second).get();
}

GPUTexture* SceneManager::get_gpu_pending_texture(UUID textureUUID) const
{
    for (const GPUPendingTexture& pendingTexture : m_pendingTextures)
    {
        asset::Texture* textureAsset = pendingTexture.gpuTexture->texture_asset();
        if (textureAsset->get_uuid() == textureUUID)
            return pendingTexture.gpuTexture.get();
    }

    return nullptr;
}

GPUMaterial* SceneManager::get_gpu_material(UUID materialUUID) const
{
    auto it = m_gpuResourcesLookup.find(materialUUID);
    if (it == m_gpuResourcesLookup.end())
        return nullptr;
    return m_gpuMaterials.at(it->second).get();
}

void SceneManager::set_cmd(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd);
    
    cmd_recorder(cmd->cmdPool->queueType).set_cmd(cmd);
}

void SceneManager::allocate_storage_buffers()
{
    auto alloc = [this](
        uint64 structSize, 
        uint64 entryCount, 
        std::vector<rhi::Buffer*>& buffers,
        const std::string& debugName
    )
    {
        uint64 cpuEntriesSize = entryCount * structSize;

        if (buffers.size() < g_frameIndex + 1)
        {
            uint64 bufferSize = cpuEntriesSize > DEFAULT_GPU_BUFFER_SIZE 
                ? calc_buffer_size(0, cpuEntriesSize) : DEFAULT_GPU_BUFFER_SIZE;

            buffers.push_back(create_uma_storage_buffer(bufferSize));
            rhi::set_name(buffers.back(), generate_resource_name(debugName));
        }
        else
        {
            rhi::Buffer* buffer = buffers.at(g_frameIndex);
            if (buffer->size < cpuEntriesSize)
            {
                uint64 currentBufferSize = buffer->size;
                rhi::destroy_buffer(buffer);

                uint64 newSize = calc_buffer_size(currentBufferSize, cpuEntriesSize);
                buffers.at(g_frameIndex) = create_uma_storage_buffer(newSize);
                rhi::set_name(buffers.back(), generate_resource_name(debugName));
            }
        }
    };

    alloc(sizeof(ShaderModel), m_gpuModels.size(), m_modelBuffers, MODEL_BUFFER_NAME);
    alloc(sizeof(ShaderModelInstance), m_modelInstanceCount, m_modelInstanceBuffers, MODEL_INSTANCE_BUFFER_NAME);
    alloc(sizeof(ShaderMeshInstance), m_meshCount, m_meshInstanceBuffers, MESH_INSTANCE_BUFFER_NAME);
    alloc(sizeof(ShaderEntity), m_shaderEntityComponents.size(), m_shaderEntityBuffers, ENTITY_BUFFER_NAME);
    alloc(sizeof(ShaderMaterial), m_gpuMaterials.size(), m_materialBuffers, MATERIAL_BUFFER_NAME);
}

rhi::Buffer* SceneManager::get_model_buffer() const
{
    return m_modelBuffers.at(g_frameIndex);
}

rhi::Buffer* SceneManager::get_model_instance_buffer() const
{
    return m_modelInstanceBuffers.at(g_frameIndex);
}

rhi::Buffer* SceneManager::get_mesh_instance_buffer() const
{
    return m_meshInstanceBuffers.at(g_frameIndex);
}

rhi::Buffer* SceneManager::get_material_buffer() const
{
    return m_materialBuffers.at(g_frameIndex);
}

rhi::Buffer* SceneManager::get_shader_entity_buffer() const
{
    return m_shaderEntityBuffers.at(g_frameIndex);
}

uint64 SceneManager::calc_buffer_size(uint64 currentSize, uint64 cpuEntrieSize)
{
    if (currentSize * 2 < cpuEntrieSize)
        return cpuEntrieSize * 2;

    return currentSize * 2;
}

void SceneManager::fill_frame_data()
{
    if (m_frameBuffers.size() < g_frameIndex + 1)
    {
        m_frameBuffers.push_back(create_uma_uniform_buffer(sizeof(FrameUB)));
        rhi::set_name(m_frameBuffers.back(), generate_resource_name(FRAME_DATA_BUFFER_NAME));
    }
    
    m_frameData.modelBufferIndex = get_model_buffer()->descriptorIndex;
    m_frameData.modelInstanceBufferIndex = get_model_instance_buffer()->descriptorIndex;
    m_frameData.meshInstanceBufferIndex = get_mesh_instance_buffer()->descriptorIndex;
    m_frameData.entityBufferIndex = get_shader_entity_buffer()->descriptorIndex;
    m_frameData.lightArrayCount = m_lightComponentCount;
    m_frameData.lightArrayOffset = 0;
    m_frameData.materialBufferIndex = get_material_buffer()->descriptorIndex;

    rhi::Buffer* buffer = m_frameBuffers.at(g_frameIndex);
    memcpy(buffer->mappedData, &m_frameData, sizeof(FrameUB));
    rhi::bind_uniform_buffer(buffer, g_frameIndex, UB_FRAME_SLOT, sizeof(FrameUB), 0);
}

void SceneManager::fill_camera_buffers()
{
    static constexpr uint64 shaderCameraBufferSize = sizeof(ShaderCamera) * MAX_CAMERA_COUNT;

    if (m_cameraBuffers.size() < g_frameIndex + 1)
    {
        m_cameraBuffers.push_back(create_uma_uniform_buffer(shaderCameraBufferSize));
        rhi::set_name(m_frameBuffers.back(), generate_resource_name(CAMERA_BUFFER_NAME));
    }
    
    if (!m_mainCameraEntity)
        return;

    engine::EditorCameraComponent* camera = m_mainCameraEntity->get_component<engine::EditorCameraComponent>();
    m_cameras[0].position = m_mainCameraEntity->get_position();
    m_cameras[0].view = camera->view;
    m_cameras[0].projection = camera->projection;
    m_cameras[0].viewProjection = camera->viewProjection;
    m_cameras[0].prevViewProjection = camera->prevViewProjection;
    m_cameras[0].inverseView = camera->inverseView;
    m_cameras[0].inverseProjection = camera->inverseProjection;
    m_cameras[0].inverseViewProjection = camera->inverseViewProjection;
    m_cameras[0].zNear = camera->zNear;
    m_cameras[0].zFar = camera->zFar;
    m_cameras[0].create_frustum();

    rhi::Buffer* buffer = m_cameraBuffers.at(g_frameIndex);
    memcpy(buffer->mappedData, m_cameras.data(), shaderCameraBufferSize);
    rhi::bind_uniform_buffer(buffer, g_frameIndex, UB_CAMERA_SLOT, shaderCameraBufferSize, 0);
}

void SceneManager::add_delete_handler(const DeleteHandler& deleteHandler)
{
    if (m_deleteHandlersPerFrame.size() < g_frameIndex + 1)
        m_deleteHandlersPerFrame.emplace_back();

    m_deleteHandlersPerFrame[g_frameIndex].push_back(deleteHandler);
}

rhi::Buffer* SceneManager::create_uma_storage_buffer(uint64 size) const
{
    rhi::BufferInfo bufferInfo;
    bufferInfo.bufferUsage = rhi::ResourceUsage::STORAGE_BUFFER;
    bufferInfo.memoryUsage = rhi::MemoryUsage::CPU_TO_GPU;
    bufferInfo.size = size;

    rhi::Buffer* buffer;
    rhi::create_buffer(&buffer, &bufferInfo);
    return buffer;
}

rhi::Buffer* SceneManager::create_uma_uniform_buffer(uint64 size) const
{
    rhi::BufferInfo bufferInfo;
    bufferInfo.bufferUsage = rhi::ResourceUsage::UNIFORM_BUFFER;
    bufferInfo.memoryUsage = rhi::MemoryUsage::CPU_TO_GPU;
    bufferInfo.size = size;

    rhi::Buffer* buffer;
    rhi::create_buffer(&buffer, &bufferInfo);
    return buffer;
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
            add_delete_handler([oldTLAS]()
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

    uint32 instanceCount = 0;
    for (engine::Entity* entity : m_entitiesForTLAS)
    {
        if (engine::ModelComponent* modelComponent = entity->get_component<engine::ModelComponent>())
        {
            GPUModel* gpuModel = get_gpu_model(modelComponent->get_model_uuid());
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

    uint32 copySize = instanceCount * instanceSize;
    rhi::Buffer* instanceBuffer = m_TLAS->info.tlas.instanceBuffer;
    rhi::copy_buffer(cmd, uploadBuffer, instanceBuffer, copySize, 0, 0);

    rhi::PipelineBarrier barrier(
        instanceBuffer,
        rhi::ResourceLayout::TRANSFER_SRC,
        rhi::ResourceLayout::SHADER_READ
    );

    rhi::build_acceleration_structure(cmd, m_TLAS, nullptr);
}

std::string SceneManager::generate_resource_name(const std::string& baseName) const
{
    return baseName + "_" + std::to_string(g_frameIndex);
}

}