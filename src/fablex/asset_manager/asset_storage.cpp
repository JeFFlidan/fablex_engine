#include "asset_storage.h"
#include "common.h"
#include "core/file_system/archive.h"
#include "core/task_composer.h"

namespace fe::asset
{

void AssetStorage::add_asset(Asset* asset)
{
    FE_CHECK(asset);

    if (m_assetByUUID.find(asset->get_uuid()) != m_assetByUUID.end())
    {
        FE_LOG(LogAssetManager, ERROR, "Asset with UUID {} was added earlier.", asset->get_uuid());
        return;
    }
    
    m_assetByUUID[asset->get_uuid()] = asset;
}

Asset* AssetStorage::get_asset(UUID uuid) const
{
    auto it = m_assetByUUID.find(uuid);

    if (it == m_assetByUUID.end())
        return nullptr;

    return it->second;
}

void AssetStorage::save_asset(UUID uuid) const
{
    auto it = m_assetByUUID.find(uuid);

    if (it == m_assetByUUID.end())
    {
        FE_LOG(LogAssetManager, ERROR, "Failed to find asset with UUID {}.", uuid);
        return;
    }

    Asset* asset = it->second;
    if (!asset->is_dirty())
        return;

    Archive archive;
    asset->serialize(archive);
    archive.save(asset->get_path());
}

void AssetStorage::save_assets() const
{
    TaskGroup taskGroup;

    for (auto [uuid, asset] : m_assetByUUID)
    {
        if (!asset->is_dirty() || has_flag(asset->get_flags(), AssetFlag::TRANSIENT))
            continue;

        TaskComposer::execute(taskGroup, [asset](TaskExecutionInfo)
        {
            Archive archive;
            asset->serialize(archive);
            archive.save(asset->get_path());
        });
    }

    TaskComposer::wait(taskGroup);
}

}