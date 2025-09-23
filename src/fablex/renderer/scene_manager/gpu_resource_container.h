#pragma once

#include "gpu_resource_handle.h"
#include "core/task_types.h"
#include "asset_manager/asset_manager.h"
#include "gpu_model.h"
#include "gpu_texture.h"
#include "gpu_material.h"

#include <mutex>
#include <concepts>
#include <unordered_map>

namespace fe::renderer
{

class SceneManager;

template<typename Visitor>
concept GPUResourceVisitor =
    std::invocable<Visitor, GPUModel&> &&
    std::invocable<Visitor, GPUTexture&> &&
    std::invocable<Visitor, GPUMaterial&> &&
    std::same_as<std::invoke_result_t<Visitor, GPUModel&>, bool> &&
    std::same_as<std::invoke_result_t<Visitor, GPUTexture&>, bool> &&
    std::same_as<std::invoke_result_t<Visitor, GPUMaterial&>, bool>;

class GPUResourceContainer
{
public:
    GPUResourceContainer(SceneManager* sceneManager);

    template<GPUResourceVisitor Visitor>
    void for_each(Visitor&& visitor)
    {
        for (auto it = m_resources.begin(); it != m_resources.end(); )
        {
            if (!it->second->visit(visitor))
            {
                it = m_resources.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

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
    std::unordered_map<UUID, GPUResourceHandlePtr> m_resources;

    template<typename T>
    T* resource(UUID uuid) const
    {
        std::scoped_lock<std::mutex> locker(m_resourcesMutex);

        auto it = m_resources.find(uuid);
        if (it == m_resources.end())
            return nullptr;

        return it->second->get<T>();
    }

    template<typename ResourceType>
    ResourceType* add_resource(UUID uuid)
    {
        using AssetType = ResourceType::AssetType;

        AssetType* asset = asset::AssetManager::get_asset<AssetType>(uuid);

        m_resources.emplace(
            uuid,
            create_gpu_resource_handle<ResourceType>(asset)
        );

        return m_resources.at(uuid)->template get<ResourceType>();
    }
};

}