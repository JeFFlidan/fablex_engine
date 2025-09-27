#pragma once

#include "gpu_resource_container.h"
#include "core/task_composer.h"
#include "scene_manager.h"

namespace fe::renderer
{

GPUResourceContainer::GPUResourceContainer(SceneManager* sceneManager)
    : m_sceneManager(sceneManager)
{

}

void GPUResourceContainer::process()
{
    for (auto it = m_resources.begin(); it != m_resources.end(); )
    {
        if (!it->second.upload_to_gpu(m_sceneManager))
        {
            std::scoped_lock<std::mutex> locker(m_resourcesMutex);
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
    for (auto& [uuid, handle] : m_resources)
        handle.reset();
}

GPUModel* GPUResourceContainer::add_model(UUID modelUUID, TaskGroup& taskGroup)
{
    if (GPUModel* gpuModel = model(modelUUID))
        return gpuModel;

    GPUModel* gpuModel = add_resource<GPUModel>(modelUUID);

    TaskComposer::execute(taskGroup, [this, gpuModel](TaskExecutionInfo execInfo)
    {
        gpuModel->build(m_sceneManager);
    });
    
    return gpuModel;
}

GPUMaterial* GPUResourceContainer::add_material(UUID materialUUID, TaskGroup& taskGroup)
{
    if (GPUMaterial* gpuMaterial = material(materialUUID))
        return gpuMaterial;

    GPUMaterial* gpuMaterial = add_resource<GPUMaterial>(materialUUID);

    TaskComposer::execute(taskGroup, [this, gpuMaterial](TaskExecutionInfo execInfo)
    {
        gpuMaterial->build(m_sceneManager);
    });

    return gpuMaterial;
}

GPUTexture* GPUResourceContainer::add_texture(UUID textureUUID, TaskGroup& taskGroup)
{
    if (GPUTexture* gpuTexture = texture(textureUUID))
        return gpuTexture;

    GPUTexture* gpuTexture = add_resource<GPUTexture>(textureUUID);
    gpuTexture->create();

    return gpuTexture;
}

}