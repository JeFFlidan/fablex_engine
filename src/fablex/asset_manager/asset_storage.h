#pragma once

#include "asset.h"
#include <mutex>
#include <unordered_map>

namespace fe::asset
{

class AssetStorage
{
public:
    void add_asset(Asset* asset);
    Asset* get_asset(UUID uuid) const;

    void save_asset(UUID uuid) const;
    void save_assets() const;

private:
    std::mutex m_mutex;
    std::unordered_map<UUID, Asset*> m_assetByUUID;
};

}