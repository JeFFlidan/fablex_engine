#include "scene_manager.h"
#include "renderer/globals.h"
#include "rhi/rhi.h"
#include "engine/entity/events.h"
#include "engine/components/model_component.h"
#include "engine/components/editor_camera_component.h"
#include "core/task_composer.h"
#include "core/primitives/sphere.h"
#include "asset_manager/model/model.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

SceneManager::SceneManager()
{
    // For now some buffers support only CPU_TO_GPU memory usage
    FE_CHECK(rhi::has_capability(rhi::GPUCapability::CACHE_COHERENT_UMA));

    m_gpuModels.reserve(asset::AssetPoolSize<asset::Model>::poolSize);

    EventManager::subscribe<engine::EntityCreatedEvent>([this](const engine::EntityCreatedEvent& event)
    {
        m_pendingEntities.push_back(event.get_entity());
    });
}

SceneManager::~SceneManager()
{
    for (rhi::Buffer* buffer : m_modelBuffers)
        rhi::destroy_buffer(buffer);

    for (rhi::Buffer* buffer : m_modelInstanceBuffers)
        rhi::destroy_buffer(buffer);

    for (rhi::Buffer* buffer : m_cameraBuffers)
        rhi::destroy_buffer(buffer);

    for (rhi::Buffer* buffer : m_frameBuffers)
        rhi::destroy_buffer(buffer);

    for (GPUModel* model : m_gpuModels)
        m_gpuModelAllocator.free(model);
}

void SceneManager::upload(rhi::CommandBuffer* cmd)
{
    m_cmdRecorder.set_cmd_buffer(cmd);

    BufferArray& stagingBuffers = m_stagingBuffersByFrameIndex[g_frameIndex];
    for (rhi::Buffer* buffer : stagingBuffers)
        rhi::destroy_buffer(buffer);
    stagingBuffers.clear();

    TaskGroup taskGroup;

    for (engine::Entity* entity : m_pendingEntities)
    {
        m_entities.push_back(entity);

        if (engine::ModelComponent* modelComponent = entity->get_component<engine::ModelComponent>())
        {
            ++m_modelComponentCount;

            if (m_gpuModelInfoByUUID.find(modelComponent->get_model_uuid()) != m_gpuModelInfoByUUID.end())
            {
                ++m_gpuModelInfoByUUID[modelComponent->get_model_uuid()].instanceCount;
                continue;
            }

            // If many instances were created at once, scene manager can create seceral GPUModel, so this line prevents that
            GPUModelInfo& gpuModelInfo = m_gpuModelInfoByUUID[modelComponent->get_model_uuid()];
            ++gpuModelInfo.instanceCount;

            // TEMP!!!! Must be done using TaskGorup with LOW priority 
            TaskComposer::execute(taskGroup, [this, modelComponent](TaskExecutionInfo execInfo)
            {
                asset::Model* model = modelComponent->get_model();
                FE_CHECK(model);

                GPUModel* gpuModel = m_gpuModelAllocator.allocate(model);
                gpuModel->build(this);

                std::scoped_lock<std::mutex> locker(m_gpuModelMutex);
                m_gpuModels.push_back(gpuModel);
                m_gpuModelInfoByUUID[model->get_uuid()].index = m_gpuModels.size() - 1;
            });
        }

        if (engine::EditorCameraComponent* cameraComponent = entity->get_component<engine::EditorCameraComponent>())
        {
            m_mainCameraEntity = entity;
        }
    }

    m_pendingEntities.clear();

    TaskComposer::wait(taskGroup);

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        rhi::Buffer* buffer = get_model_buffer();
        ShaderModel* shaderModels = static_cast<ShaderModel*>(buffer->mappedData);
        uint64 index = 0;

        for (GPUModel* gpuModel : m_gpuModels)
        {
            ShaderModel& shaderModel = shaderModels[index++];
            shaderModel.init();
            gpuModel->fill_shader_model(shaderModel);
        }
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        rhi::Buffer* buffer = get_model_instance_buffer();
        ShaderModelInstance* shaderModelInstances = static_cast<ShaderModelInstance*>(buffer->mappedData);
        uint64 index = 0;

        for (engine::Entity* entity : m_entities)
        {
            if (engine::ModelComponent* modelComponent = entity->get_component<engine::ModelComponent>())
            {
                const GPUModelInfo& gpuModelInfo = m_gpuModelInfoByUUID.at(modelComponent->get_model_uuid());
                GPUModel* gpuModel = m_gpuModels.at(gpuModelInfo.index);

                ShaderModelInstance& modelInstance = shaderModelInstances[index++];
                modelInstance.init();
                modelInstance.geometryOffset = gpuModelInfo.index;

                Sphere sphereBounds(gpuModel->get_aabb());
                modelInstance.sphereBounds.center = sphereBounds.center;
                modelInstance.sphereBounds.radius = sphereBounds.radius;

                Matrix remapMat = gpuModel->get_aabb().get_unorm_remap_matrix();
                Matrix transformMat = entity->get_world_transform();

                modelInstance.scale = entity->get_scale();
                modelInstance.transform.set_transfrom(remapMat * transformMat);
                modelInstance.rawTransform.set_transfrom(entity->get_world_transform());
                modelInstance.transformInverseTranspose.set_transfrom(transformMat.transpose().inverse());
            }
        }
    });

    TaskComposer::execute(taskGroup, [this](TaskExecutionInfo execInfo)
    {
        fill_frame_data();
        fill_camera_buffers();
    });

    TaskComposer::wait(taskGroup);
}

uint32 SceneManager::get_model_index(GPUModel* gpuModel) const
{
    auto it = m_gpuModelInfoByUUID.find(gpuModel->get_model()->get_uuid());
    if (it == m_gpuModelInfoByUUID.end())
        return ~0u;
    return it->second.index;
}

uint32 SceneManager::get_instance_count(GPUModel* gpuModel) const
{
    auto it = m_gpuModelInfoByUUID.find(gpuModel->get_model()->get_uuid());
    if (it == m_gpuModelInfoByUUID.end())
        return ~0u;
    return it->second.instanceCount;
}

void SceneManager::add_staging_buffer(rhi::Buffer* buffer)
{
    FE_CHECK(buffer);
    m_stagingBuffersByFrameIndex[g_frameIndex].push_back(buffer);
}

rhi::Buffer* SceneManager::get_model_buffer()
{
    if (m_modelBuffers.size() < g_frameIndex + 1)
        m_modelBuffers.push_back(create_uma_storage_buffer());

    rhi::Buffer* buffer = m_modelBuffers.at(g_frameIndex);
    uint64 cpuEntriesSize = sizeof(ShaderModel) * m_gpuModelInfoByUUID.size();

    if (buffer->size < cpuEntriesSize)
    {
        rhi::destroy_buffer(buffer);

        uint64 newSize = calc_buffer_size(buffer->size, cpuEntriesSize);
        m_modelBuffers.at(g_frameIndex) = create_uma_storage_buffer(newSize);
    }

    return m_modelBuffers.at(g_frameIndex);
}

rhi::Buffer* SceneManager::get_model_instance_buffer()
{
    if (m_modelInstanceBuffers.size() < g_frameIndex + 1)
        m_modelInstanceBuffers.push_back(create_uma_storage_buffer());

    rhi::Buffer* buffer = m_modelInstanceBuffers.at(g_frameIndex);
    uint64 cpuEntriesSize = sizeof(ShaderModelInstance) * m_modelComponentCount;

    if (buffer->size < cpuEntriesSize)
    {
        rhi::destroy_buffer(buffer);

        uint64 newSize = calc_buffer_size(buffer->size, cpuEntriesSize);
        m_modelInstanceBuffers.at(g_frameIndex) = create_uma_storage_buffer(newSize);
    }

    return m_modelInstanceBuffers.at(g_frameIndex);
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
        m_frameBuffers.push_back(create_uma_uniform_buffer(sizeof(FrameUB)));
    
    m_frameData.modelBufferIndex = get_model_buffer()->descriptorIndex;
    m_frameData.modelInstanceBufferIndex = get_model_instance_buffer()->descriptorIndex;

    rhi::Buffer* buffer = m_frameBuffers.at(g_frameIndex);
    memcpy(buffer->mappedData, &m_frameData, sizeof(FrameUB));
    rhi::bind_uniform_buffer(buffer, g_frameIndex, UB_FRAME_SLOT, sizeof(FrameUB), 0);
}

void SceneManager::fill_camera_buffers()
{
    static constexpr uint64 shaderCameraBufferSize = sizeof(ShaderCamera) * MAX_CAMERA_COUNT;

    if (m_cameraBuffers.size() < g_frameIndex + 1)
        m_cameraBuffers.push_back(create_uma_uniform_buffer(shaderCameraBufferSize));
    
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

}