#include "asset_registry.h"
#include "common.h"
#include "core/file_system/file_system.h"
#include "core/file_system/archive.h"

namespace fe::asset
{

void AssetRegistry::init()
{
    FE_LOG(LogAssetManager, INFO, "Starting AssetRegistry initialization.");

    for (auto [uuid, assetData] : s_assetDataByUUID)
        s_assetDataPool.free(assetData);
    
    s_assetDataByUUID.clear();
    s_assetCounters.resize(get_asset_type_count(), 0);

    FileSystem::for_each_file(
        FileSystem::get_project_path(), 
        {"feasset"},
        [](const DirectoryEntry& dirEntry)
        {
            std::string filePath = dirEntry.path().string();
            Archive archive(filePath, Archive::Mode::READ_HEADER_ONLY);
            FE_LOG(LogDefault, INFO, "UUID: {}", archive.get_uuid());
            if (is_uuid_ambiguous(archive.get_uuid(), filePath))
                return;

            AssetData* assetData = s_assetDataPool.allocate();
            assetData->name = FileSystem::get_file_name(filePath);
            assetData->path = filePath;
            assetData->uuid = archive.get_uuid();
            assetData->type = archive.get_object_type<Type>();
            FE_LOG(LogDefault, INFO, "AssetType: {}", (uint32)assetData->type);
            s_assetDataByUUID[assetData->uuid] = assetData;
            s_assetDataByPath[assetData->path] = assetData;
        }
    );

    FE_LOG(LogAssetManager, INFO, "AssetRegistry initialization completed.");
}

void AssetRegistry::register_asset(Asset* asset)
{
    std::scoped_lock<std::mutex> locker(s_mutex);

    if (is_uuid_ambiguous(asset->get_uuid(), asset->get_path()))
        return;

    AssetData* assetData = s_assetDataPool.allocate();
    assetData->name = asset->get_name();
    assetData->path = asset->get_path();
    assetData->uuid = asset->get_uuid();
    assetData->type = asset->get_type();

    s_assetDataByUUID[assetData->uuid] = assetData;
    s_assetDataByPath[assetData->path] = assetData;

    increase_type_counter(asset->get_type());
}

void AssetRegistry::unregister_asset(UUID uuid)
{
    std::scoped_lock<std::mutex> locker(s_mutex);

    auto it = s_assetDataByUUID.find(uuid);
    if (it == s_assetDataByUUID.end())
    {
        FE_LOG(LogAssetManager, ERROR, "AssetRegistry::unregister_asset(): Failed to find asset with UUID {}.", uuid);
        return;
    }

    decrease_type_counter(it->second->type);

    s_assetDataByPath.erase(s_assetDataByPath.find(it->second->path));
    s_assetDataPool.free(it->second);
    s_assetDataByUUID.erase(it);
}

void AssetRegistry::get_assets_by_type(Type inAssetType, std::vector<const AssetData*>& outAssets)
{
    std::scoped_lock<std::mutex> locker(s_mutex);

    outAssets.reserve(s_assetCounters.at(std::to_underlying(inAssetType)));

    for (auto [uuid, assetData] : s_assetDataByUUID)
        if (assetData->type == inAssetType)
            outAssets.push_back(assetData);
}

const AssetData* AssetRegistry::get_asset_data_by_uuid(UUID uuid)
{
    std::scoped_lock<std::mutex> locker(s_mutex);

    auto it = s_assetDataByUUID.find(uuid);
    if (it == s_assetDataByUUID.end())
    {
        FE_LOG(LogAssetManager, ERROR, "AssetRegistry::get_asset_data_by_uuid(): Failed to find asset with UUID {}.", uuid);
        return nullptr;
    }

    return it->second;
}

const AssetData* AssetRegistry::get_asset_data_by_path(const std::string& assetPath)
{
    std::scoped_lock<std::mutex> locker(s_mutex);

    auto it = s_assetDataByPath.find(assetPath);
    if (it == s_assetDataByPath.end())
    {
        FE_LOG(LogAssetManager, ERROR, "AssetRegistry::get_asset_data_by_path(): Failed to find asset with path {}.", assetPath);
        return nullptr;
    }

    return it->second;
}

bool AssetRegistry::is_uuid_ambiguous(UUID uuid, const std::string& assetPath)
{
    auto it = s_assetDataByUUID.find(uuid);

    if (it != s_assetDataByUUID.end())
    {
        FE_LOG(LogAssetManager, ERROR, "Ambiguous UUID at {} and {}", it->second->path, assetPath);
        return true;
    }

    return false;
}

void AssetRegistry::increase_type_counter(Type assetType)
{
    FE_CHECK(assetType != Type::COUNT);
    ++s_assetCounters.at(std::to_underlying(assetType));
}

void AssetRegistry::decrease_type_counter(Type assetType)
{
    FE_CHECK(assetType != Type::COUNT);
    --s_assetCounters.at(std::to_underlying(assetType));
}

}
