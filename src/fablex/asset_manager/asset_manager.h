#pragma once

#include "common.h"
#include "events.h"
#include "asset_storage.h"
#include "asset_registry.h"
#include "model/model.h"
#include "texture/texture.h"
#include "core/pool_allocator.h"
#include "core/file_system/archive.h"

namespace fe::asset
{

// TODO: Think about asset loading using Task Composer and low priority TaskGroup
class AssetManager
{
public:
    static void init();

    static Model* create_model(const ModelCreateInfo& createInfo);
    static Texture* create_texture(const TextureCreateInfo& createInfo);
    static Material* create_material(const MaterialCreateInfo& createInfo);

    static bool import_model(const ModelImportContext& inImportContext, ModelImportResult& outImportResult);
    static bool import_texture(const TextureImportContext& inImportContext, TextureImportResult& outImportResult);

    static Model* get_model(UUID uuid);
    static Texture* get_texture(UUID uuid);
    static Material* get_material(UUID uuid);

    template<typename T>
    static T* get_asset(UUID uuid)
    {
        FE_COMPILE_CHECK((std::is_base_of_v<Asset, T>));

        if (T* asset = static_cast<T*>(s_assetStorage.get_asset(uuid)))
            return asset;

        const AssetData* assetData = AssetRegistry::get_asset_data_by_uuid(uuid);
        if (!assetData)
            return nullptr;

        Archive archive(assetData->path);
        T* asset = allocate<T>();
        asset->deserialize(archive);

        s_assetStorage.add_asset(asset);

        EventManager::enqueue_event(AssetLoadedEvent<T>(asset));

        return asset;
    }

    static bool is_asset_loaded(UUID assetUUID);

    static void save_assets();
    static void save_asset(UUID uuid);

    static bool is_model_format_supported(const std::string& extension);
    static bool is_texture_format_supported(const std::string& extension);

private:
    inline static AssetStorage s_assetStorage;

    template<typename T>
    static T* allocate()
    {
        FE_COMPILE_CHECK((std::is_base_of_v<Asset, T>));
        static ThreadSafePoolAllocator<T, AssetPoolSize<T>::poolSize> allocator;
        return allocator.allocate();
    }

    static void configure_imported_asset(Asset* asset, const ImportContext& importContext);
    static void configure_created_asset(Asset* asset, const CreateInfo& createInfo);
    static std::string generate_path(const std::string& projectDirectory, const std::string& name);
};

}