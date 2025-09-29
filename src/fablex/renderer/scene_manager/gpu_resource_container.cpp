#pragma once

#include "gpu_resource_container.h"
#include "gpu_resource_counters.h"
#include "core/task_composer.h"
#include "scene_manager.h"

namespace fe::renderer
{

GPUResourceContainer::GPUResourceContainer(SceneManager* sceneManager)
    : m_sceneManager(sceneManager)
{

}

GPUResourceContainer::~GPUResourceContainer()
{
    for (auto [uuid, resource] : m_resources)
    {
        GPUResourceFreeHandler freeHandler = resource->free_handler();
        freeHandler(resource);
    }
}

void GPUResourceContainer::process()
{
    for (auto it = m_resources.begin(); it != m_resources.end();)
    {
        GPUResourceBase* resource = it->second;

        if (!resource->upload_to_gpu(m_sceneManager))
        {
            std::scoped_lock<std::mutex> locker(m_resourcesMutex);
            
            GPUResourceFreeHandler freeHandler = resource->free_handler();
            freeHandler(resource);

            it = m_resources.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void GPUResourceContainer::reset()
{
    for (auto& [uuid, resource] : m_resources)
        resource->reset();
}

GPUModel* GPUResourceContainer::add_model(UUID modelUUID, TaskGroup& taskGroup)
{
    if (GPUModel* gpuModel = model(modelUUID))
    {
        if (gpuModel->ref_count() == 0)
            gpuModel->index(GPUResourceCounters::next_model_index());

        return gpuModel;
    }

    GPUModel* gpuModel = add_resource<GPUModel>(modelUUID);

    TaskComposer::execute(taskGroup, [this, gpuModel](TaskExecutionInfo execInfo)
    {
        gpuModel->build(m_sceneManager);
    });

    gpuModel->index(GPUResourceCounters::next_model_index());
    
    return gpuModel;
}

GPUMaterial* GPUResourceContainer::add_material(UUID materialUUID, TaskGroup& taskGroup)
{
    if (GPUMaterial* gpuMaterial = material(materialUUID))
    {
        if (gpuMaterial->ref_count() == 0)
            gpuMaterial->index(GPUResourceCounters::next_material_index());

        return gpuMaterial;
    }

    GPUMaterial* gpuMaterial = add_resource<GPUMaterial>(materialUUID);

    TaskComposer::execute(taskGroup, [this, gpuMaterial](TaskExecutionInfo execInfo)
    {
        gpuMaterial->build(m_sceneManager);
    });

    gpuMaterial->index(GPUResourceCounters::next_material_index());

    return gpuMaterial;
}

GPUTexture* GPUResourceContainer::add_texture(UUID textureUUID, TaskGroup& taskGroup)
{
    if (GPUTexture* gpuTexture = texture(textureUUID))
        return gpuTexture;

    GPUTexture* gpuTexture = add_resource<GPUTexture>(textureUUID);
    gpuTexture->build(m_sceneManager);

    return gpuTexture;
}

}