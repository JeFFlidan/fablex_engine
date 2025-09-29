#pragma once

#include "core/task_types.h"
#include "asset_manager/asset_manager.h"
#include "gpu_model.h"
#include "gpu_texture.h"
#include "gpu_material.h"

#include <mutex>
#include <unordered_map>

namespace fe::renderer
{

class SceneManager;

class GPUResourceContainer
{
public:
    GPUResourceContainer(SceneManager* sceneManager);
    ~GPUResourceContainer();

    void process();
    void reset();

    GPUModel* add_model(UUID modelUUID, TaskGroup& taskGroup);
    GPUMaterial* add_material(UUID materialUUID, TaskGroup& taskGroup);
    GPUTexture* add_texture(UUID textureUUID, TaskGroup& taskGroup);

    GPUModel* model(UUID uuid) const { return resource<GPUModel>(uuid); }
    GPUTexture* texture(UUID uuid) const { return resource<GPUTexture>(uuid); }
    GPUMaterial* material(UUID uuid) const { return resource<GPUMaterial>(uuid); }

private:
    SceneManager* m_sceneManager;

    mutable std::mutex m_resourcesMutex;
    std::unordered_map<UUID, GPUResourceBase*> m_resources;

    template<typename ResourceType>
    ResourceType* resource(UUID uuid) const
    {
        std::scoped_lock<std::mutex> locker(m_resourcesMutex);

        auto it = m_resources.find(uuid);
        if (it == m_resources.end())
            return nullptr;

        return static_cast<ResourceType*>(it->second);
    }

    template<typename ResourceType>
    ResourceType* add_resource(UUID uuid)
    {
        using AssetType = ResourceType::AssetType;

        AssetType* asset = asset::AssetManager::get_asset<AssetType>(uuid);
        m_resources.emplace(uuid, GPUResourceAllocator<ResourceType>::allocate(asset));
        return static_cast<ResourceType*>(m_resources.at(uuid));
    }
};

}