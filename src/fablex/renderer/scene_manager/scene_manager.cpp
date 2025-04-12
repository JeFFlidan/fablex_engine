#include "scene_manager.h"
#include "renderer/globals.h"
#include "rhi/rhi.h"
#include "engine/entity/events.h"
#include "engine/components/model_component.h"
#include "core/task_composer.h"
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

    m_gpuModelIndexByUUID.clear();
}

void SceneManager::upload(rhi::CommandBuffer* cmd)
{
    TaskGroup taskGroup;

    for (engine::Entity* entity : m_pendingEntities)
    {
        m_entities.push_back(entity);

        if (engine::ModelComponent* modelComponent = entity->get_component<engine::ModelComponent>())
        {
            ++m_modelComponentCount;

            if (m_gpuModelIndexByUUID.find(modelComponent->get_model_uuid()) != m_gpuModelIndexByUUID.end())
                continue;

            // TEMP!!!! Must be done using TaskGorup with LOW priority 
            TaskComposer::execute(taskGroup, [this, modelComponent, cmd](TaskExecutionInfo execInfo)
            {
                asset::Model* model = modelComponent->get_model();
                FE_CHECK(model);

                GPUModel* gpuModel = m_gpuModelAllocator.allocate(model);
                gpuModel->build(cmd);

                m_gpuModels.push_back(gpuModel);
                m_gpuModelIndexByUUID[model->get_uuid()] = m_gpuModels.size() - 1;
            });
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
            entity->update_world_transform();

            if (engine::ModelComponent* modelComponent = entity->get_component<engine::ModelComponent>())
            {
                GPUModel* gpuModel = m_gpuModels.at(m_gpuModelIndexByUUID.at(modelComponent->get_model_uuid()));

                ShaderModelInstance& modelInstance = shaderModelInstances[index++];
                modelInstance.init();
                modelInstance.geometryOffest = m_gpuModelIndexByUUID.at(modelComponent->get_model_uuid());

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

    TaskComposer::wait(taskGroup);
}

rhi::Buffer* SceneManager::get_model_buffer()
{
    if (m_modelBuffers.size() > g_frameIndex + 1)
        m_modelBuffers.push_back(create_uma_buffer());

    rhi::Buffer* buffer = m_modelBuffers.at(g_frameIndex);
    uint64 cpuEntriesSize = sizeof(ShaderModel) * m_gpuModelIndexByUUID.size();
    
    if (buffer->size < cpuEntriesSize)
    {
        rhi::destroy_buffer(buffer);

        uint64 newSize = calc_buffer_size(buffer->size, cpuEntriesSize);
        m_modelBuffers.at(g_frameIndex) = create_uma_buffer(newSize);
    }

    return m_modelBuffers.at(g_frameIndex);
}

rhi::Buffer* SceneManager::get_model_instance_buffer()
{
    if (m_modelInstanceBuffers.size() > g_frameIndex + 1)
        m_modelInstanceBuffers.push_back(create_uma_buffer());

    rhi::Buffer* buffer = m_modelInstanceBuffers.at(g_frameIndex);
    uint64 cpuEntriesSize = sizeof(ShaderModelInstance) * m_modelComponentCount;

    if (buffer->size < cpuEntriesSize)
    {
        rhi::destroy_buffer(buffer);

        uint64 newSize = calc_buffer_size(buffer->size, cpuEntriesSize);
        m_modelInstanceBuffers.at(g_frameIndex) = create_uma_buffer(newSize);
    }

    return m_modelInstanceBuffers.at(g_frameIndex);
}

uint64 SceneManager::calc_buffer_size(uint64 currentSize, uint64 cpuEntrieSize)
{
    if (currentSize * 2 < cpuEntrieSize)
        return cpuEntrieSize * 2;

    return currentSize * 2;
}

rhi::Buffer* SceneManager::create_uma_buffer(uint64 size) const
{
    rhi::BufferInfo bufferInfo;
    bufferInfo.bufferUsage = rhi::ResourceUsage::STORAGE_BUFFER;
    bufferInfo.memoryUsage = rhi::MemoryUsage::CPU_TO_GPU;
    bufferInfo.size = DEFAULT_GPU_BUFFER_SIZE;

    rhi::Buffer* buffer;
    rhi::create_buffer(&buffer, &bufferInfo);
    return buffer;
}

}