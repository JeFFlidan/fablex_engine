#pragma once

#include "asset_manager/fwd.h"
#include "asset_manager/enums.h"
#include <unordered_map>

namespace fe::editor
{

enum class ContentFolder 
{
    ROOT,
    MATERIALS,
    MODELS,
    TEXTURES
};

class ContentBrowser
{
public:
    ContentBrowser();

    void draw();

private:
    asset::Texture* m_folderIcon = nullptr;
    const asset::AssetData* m_renamingAsset = nullptr;
    char m_renameBuffer[256] = {};

    std::unordered_map<asset::Type, asset::Texture*> m_iconByAssetType;

    ContentFolder m_currentFolder = ContentFolder::ROOT;

    void load_icons();
};

}