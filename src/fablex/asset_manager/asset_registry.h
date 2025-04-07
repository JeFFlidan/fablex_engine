#pragma once

#include "asset.h"
#include "core/uuid.h"
#include "core/pool_allocator.h"
#include <string>
#include <unordered_set>

namespace fe::asset
{

struct AssetData
{
    UUID uuid;
    Type type;
    std::string path;
    std::string name;
};

class AssetRegistry
{
public:
    static void init();
    static void register_asset(Asset* asset);
    static void unregister_asset(UUID uuid);
    static void get_assets_by_type(Type inAssetType, std::vector<const AssetData*>& outAssets);
    static const AssetData* get_asset_data_by_uuid(UUID uuid);

    // The path can be either relative or absolute 
    static const AssetData* get_asset_data_by_path(const std::string& assetPath);

private:
    inline static std::mutex s_mutex;
    inline static PoolAllocator<AssetData, 1024> s_assetDataPool;
    inline static std::unordered_map<UUID, AssetData*> s_assetDataByUUID;
    inline static std::unordered_map<std::string, AssetData*> s_assetDataByPath;
    inline static std::vector<uint64> s_assetCounters;

    static bool is_uuid_ambiguous(UUID uuid, const std::string& assetPath);
    static void increase_type_counter(Type assetType);
    static void decrease_type_counter(Type assetType);
};
    
}