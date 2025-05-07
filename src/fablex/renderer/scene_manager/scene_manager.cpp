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
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

constexpr uint64 SHADER_ENTITY_INIT_COUNT = 1024ULL;
constexpr uint64 MATERIAL_INIT_COUNT = 256;

const std::string MODEL_BUFFER_NAME = "ModelBuffer";
const std::string MODEL_INSTANCE_BUFFER_NAME = "ModelInstanceBuffer";
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
    load_resources();
    create_samplers();
}

SceneManager::~SceneManager()
{
    for (rhi::Buffer* buffer : m_modelBuffers)
        rhi::destroy_buffer(buffer);

    for (rhi::Buffer* buffer : m_modelInstanceBuffers)
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
    m_gpuTextureByUUID.clear();

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
            m_modelComponents.push_back(modelComponent);
            m_entitiesForTLAS.push_back(entity);

            if (m_gpuModelInfoByUUID.find(modelComponent->get_model_uuid()) != m_gpuModelInfoByUUID.end())
            {
                ++m_gpuModelInfoByUUID[modelComponent->get_model_uuid()].instanceCount;
            }
            else
            {
                asset::Model* model = modelComponent->get_model();
                uint64 index = m_gpuModels.size();
                m_gpuModels.emplace_back(model);

                GPUModelInfo& gpuModelInfo = m_gpuModelInfoByUUID[model->get_uuid()];
                gpuModelInfo.index = index;
                ++gpuModelInfo.instanceCount;
                
                // TEMP!!!! Must be done using TaskGorup with LOW priority 
                TaskComposer::execute(taskGroup, [this, index](TaskExecutionInfo execInfo)
                {
                    m_gpuModels[index].build(this, get_cmd_recorder(rhi::QueueType::GRAPHICS));
                });
            }

        }

        if (engine::MaterialComponent* materialComponent = entity->get_component<engine::MaterialComponent>())
        {
            UUID materialUUID = materialComponent->get_material_uuid();
            m_materialComponents.push_back(materialComponent);

            if (m_gpuMaterialInfoByUUID.find(materialUUID) == m_gpuMaterialInfoByUUID.end())
            {
                uint64 index = m_gpuMaterials.size();
                m_gpuMaterials.emplace_back(materialComponent);
                m_gpuMaterialInfoByUUID[materialUUID].index = index;

                m_gpuMaterialInfoByUUID[materialComponent->get_material_uuid()];

                TaskComposer::execute(taskGroup, [this, index](TaskExecutionInfo execInfo)
                {
                    m_gpuMaterials[index].build(this, get_cmd_recorder(rhi::QueueType::GRAPHICS));
                });
            }
        }

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

    m_pendingEntities.clear();

    TaskComposer::wait(taskGroup);

    FE_CHECK(m_materialComponents.size() == m_modelComponents.size());

    allocate_storage_buffers();

    // More offsets will be added further when new ShaderEntities will be created
    m_lightEntityBufferOffset = 0;

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        for (auto& [texture, initInfo] : m_pendingTextureCreateInfos)
        {
            m_gpuTextureByUUID.at(texture->get_uuid())->build(get_cmd_recorder(rhi::QueueType::GRAPHICS), initInfo);
        }
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        rhi::Buffer* buffer = get_model_buffer();
        ShaderModel* shaderModels = static_cast<ShaderModel*>(buffer->mappedData);
        uint64 index = 0;

        for (const GPUModel& gpuModel : m_gpuModels)
        {
            ShaderModel& shaderModel = shaderModels[index++];
            shaderModel.init();
            gpuModel.fill_shader_model(shaderModel);
        }
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        rhi::Buffer* buffer = get_model_instance_buffer();
        ShaderModelInstance* shaderModelInstances = static_cast<ShaderModelInstance*>(buffer->mappedData);
        uint64 index = 0;

        auto getGPUMaterialInfo = [this](uint64 index)->const GPUMaterialInfo&
        {
            const engine::MaterialComponent* material = m_materialComponents.at(index);
            return m_gpuMaterialInfoByUUID[material->get_material_uuid()];
        };

        for (engine::ModelComponent* modelComponent : m_modelComponents)
        {
            const GPUModelInfo& gpuModelInfo = m_gpuModelInfoByUUID.at(modelComponent->get_model_uuid());
            const GPUMaterialInfo& gpuMaterialInfo = getGPUMaterialInfo(index);

            ShaderModelInstance& modelInstance = shaderModelInstances[index++];
            modelInstance.init();
            
            modelInstance.geometryOffset = gpuModelInfo.index;
            modelInstance.materialIndex = gpuMaterialInfo.index;

            modelComponent->fill_shader_instance_data(modelInstance);
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
        
        for (const GPUMaterial& material : m_gpuMaterials)
        {
            ShaderMaterial& shaderMaterial = shaderMaterials[index++];
            shaderMaterial.init();
            material.fill_shader_material(this, shaderMaterial);
        }
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        fill_frame_data();
        fill_camera_buffers();
    });

    TaskComposer::wait(taskGroup);

    for (auto& [texture, initInfo] : m_pendingTextureCreateInfos)
    {
        rhi::Buffer* buffer = initInfo.buffer;
        add_delete_handler([buffer](){ rhi::destroy_buffer(buffer); });
    }

    m_pendingTextureCreateInfos.clear();
}

void SceneManager::build_bvh(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd->cmdPool->queueType == rhi::QueueType::COMPUTE);

    set_cmd(cmd);

    for (GPUModel& gpuModel : m_gpuModels)
        gpuModel.build_blas(get_cmd_recorder(rhi::QueueType::COMPUTE));

    fill_tlas(cmd);
}

bool SceneManager::is_texture_loaded_into_gpu(UUID textureUUID) const
{
    return false;
}

void SceneManager::load_texture_into_gpu(UUID textureUUID) const
{

}

uint32 SceneManager::get_model_index(const GPUModel& gpuModel) const
{
    auto it = m_gpuModelInfoByUUID.find(gpuModel.get_model()->get_uuid());
    if (it == m_gpuModelInfoByUUID.end())
        return ~0u;
    return it->second.index;
}

uint32 SceneManager::get_instance_count(const GPUModel& gpuModel) const
{
    auto it = m_gpuModelInfoByUUID.find(gpuModel.get_model()->get_uuid());
    if (it == m_gpuModelInfoByUUID.end())
        return ~0u;
    return it->second.instanceCount;
}

int32 SceneManager::get_descriptor(asset::Texture* texture) const
{
    auto it = m_gpuTextureByUUID.find(texture->get_uuid());
    if (it == m_gpuTextureByUUID.end())
        return -1;
    return it->second->get_gpu_texture_view()->descriptorIndex;
}

int32 SceneManager::get_sampler_descriptor(ResourceName samplerName) const
{
    auto it = m_samplerByName.find(samplerName);
    if (it == m_samplerByName.end())
        return -1;
    return it->second->descriptorIndex;
}

const GPUTexture& SceneManager::get_blue_noise_texture() const
{
    auto it = m_gpuTextureByUUID.find(m_blueNoiseTextureUUID);
    if (it == m_gpuTextureByUUID.end())
        FE_LOG(LogRenderer, FATAL, "Blue noise texture is not loaded");
    return *it->second.get();
}

void SceneManager::add_staging_buffer(rhi::Buffer* buffer)
{
    FE_CHECK(buffer);
    
    add_delete_handler([buffer]()
    {
        rhi::destroy_buffer(buffer);
    });
}

const CommandRecorder& SceneManager::get_cmd_recorder(rhi::QueueType queueType) const
{
    return *m_cmdRecorderPerQueue.at(rhi::get_queue_index(queueType));
}

void SceneManager::allocate_arrays()
{
    m_gpuModels.reserve(asset::AssetPoolSize<asset::Model>::poolSize);
    m_shaderEntityComponents.reserve(SHADER_ENTITY_INIT_COUNT);
    m_modelComponents.reserve(asset::AssetPoolSize<asset::Model>::poolSize * 4);
    m_gpuMaterials.reserve(MATERIAL_INIT_COUNT);
    m_materialComponents.reserve(asset::AssetPoolSize<asset::Model>::poolSize * 4);

    for (uint32 i = 0; i != rhi::g_queueCount; ++i)
        m_cmdRecorderPerQueue.emplace_back(new CommandRecorder());
}

void SceneManager::init_callbacks()
{
    EventManager::subscribe<engine::EntityCreatedEvent>([this](const engine::EntityCreatedEvent& event)
    {
        m_pendingEntities.push_back(event.get_entity());
    });

    EventManager::subscribe<asset::CopyTextureIntoGPURequest>([this](const asset::CopyTextureIntoGPURequest& event)
    {
        asset::Texture* texture = event.get_texture();
        const rhi::TextureInitInfo& initInfo = event.get_texture_init_info();

        GPUTexture* gpuTexture = nullptr;

        {
            std::scoped_lock<std::mutex> locker(m_textureMutex);
            GPUTextureCreateInfo& createInfo = m_pendingTextureCreateInfos.emplace_back();
            createInfo.textureAsset = texture;
            createInfo.initInfo = initInfo;
            m_gpuTextureByUUID.emplace(texture->get_uuid(), new GPUTexture(texture));
            gpuTexture = m_gpuTextureByUUID.at(texture->get_uuid()).get();
        }

        gpuTexture->create(initInfo.mipMaps.size());
    });
}

void SceneManager::load_resources()
{
    asset::TextureImportContext blueNoiseImportContext;
    blueNoiseImportContext.originalFilePath = "content/BlueNoise3DIndependent.dds";
    blueNoiseImportContext.projectDirectory = " ";

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

void SceneManager::set_cmd(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd);
    
    get_cmd_recorder(cmd->cmdPool->queueType).set_cmd(cmd);
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

    alloc(sizeof(ShaderModel), m_gpuModelInfoByUUID.size(), m_modelBuffers, MODEL_BUFFER_NAME);
    alloc(sizeof(ShaderModelInstance), m_modelComponents.size(), m_modelInstanceBuffers, MODEL_INSTANCE_BUFFER_NAME);
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
    uint64 objectCount = m_entitiesForTLAS.size() * 2;
    
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
            m_uploadBuffersForTLAS.emplace_back();

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
            const GPUModelInfo& gpuModelInfo = m_gpuModelInfoByUUID[modelComponent->get_model_uuid()];

            rhi::TLAS::Instance instance;
            instance.instanceID = instanceCount;

            instance.blas = m_gpuModels.at(gpuModelInfo.index).get_BLASes().at(0);
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