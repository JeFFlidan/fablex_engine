#include "content_browser.h"
#include "material_window.h"
#include "events.h"

#include "asset_manager/asset_manager.h"
#include "asset_manager/material/opaque_material_settings.h"

#include "core/task_composer.h"
#include "core/file_system/file_system.h"

#include "renderer/utils.h"

#include "imgui.h"
#include <vector>
#include <string>

namespace fe::editor
{

ContentBrowser::ContentBrowser()
{
    
}

void ContentBrowser::draw()
{
    if (m_iconByAssetType.empty())
        load_icons();

    ImGui::Begin("Content Browser");

    // Back Button
    if (m_currentFolder != ContentFolder::ROOT)
    {
        if (ImGui::Button("<- Back"))
        {
            m_currentFolder = ContentFolder::ROOT;
        }
        ImGui::Separator();
    }

    const float thumbnailSize = 80.0f;
    const float padding = 16.0f;
    float cellSize = thumbnailSize + padding;
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    ImGui::Columns(columnCount, nullptr, false);

    if (m_currentFolder == ContentFolder::ROOT)
    {
        const std::vector<std::pair<const char*, ContentFolder>> rootItems = 
        {
            {"Materials", ContentFolder::MATERIALS},
            {"Models", ContentFolder::MODELS},
            {"Textures", ContentFolder::TEXTURES},
        };

        uint32 folderIconDescriptor = renderer::Utils::get_descriptor(m_folderIcon);

        for (auto& [name, folder] : rootItems)
        {
            ImGui::PushID(name);

            ImGui::ImageButton(name, folderIconDescriptor, ImVec2(thumbnailSize, thumbnailSize));
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                m_currentFolder = folder;
            }

            ImGui::TextWrapped("%s", name);
            ImGui::NextColumn();

            ImGui::PopID();
        }
    }
    else
    {
        // Example content items inside a folder
        const std::unordered_map<ContentFolder, asset::Type> assetTypeByFolder = {
            {ContentFolder::TEXTURES, asset::Type::TEXTURE},
            {ContentFolder::MODELS, asset::Type::MODEL},
            {ContentFolder::MATERIALS, asset::Type::MATERIAL}
        };

        asset::Type assetType = assetTypeByFolder.at(m_currentFolder);
        uint32 iconDescriptor = renderer::Utils::get_descriptor(m_iconByAssetType.at(assetType));
        auto allAssetData = asset::AssetRegistry::get_assets_data_by_type(assetType);

        for (const asset::AssetData* assetData : allAssetData)
        {
            ImGui::PushID(assetData->name.c_str());

            ImGui::ImageButton(assetData->name.c_str(), iconDescriptor, ImVec2(thumbnailSize, thumbnailSize));

            if (ImGui::BeginPopupContextItem("##RenamePopup"))
            {
                if (ImGui::MenuItem("Rename"))
                {
                    m_renamingAsset = assetData;
                    strcpy_s(m_renameBuffer, assetData->name.c_str());
                }
                ImGui::EndPopup();
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                switch (assetType)
                {
                case asset::Type::MATERIAL:
                {
                    asset::Material* material = asset::AssetManager::get_material(assetData->uuid);
                    FE_CHECK(material);

                    EventManager::enqueue_event(WindowCreationRequest([material]{
                        return std::make_unique<MaterialWindow>(material);
                    }));

                    break;
                }
                default:
                    break;
                }
            }

            if (m_renamingAsset == assetData)
            {
                ImGui::SetNextItemWidth(thumbnailSize);

                if (ImGui::InputText(
                    "##Rename", 
                    m_renameBuffer, 
                    sizeof(m_renameBuffer),
                    ImGuiInputTextFlags_EnterReturnsTrue)
                )
                {
                    asset::AssetManager::rename_asset(m_renamingAsset->uuid, m_renameBuffer);
                    m_renamingAsset = nullptr;
                }

                if (!ImGui::IsItemActive() && ImGui::IsItemDeactivated())
                    m_renamingAsset = nullptr;

                if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                    m_renamingAsset = nullptr;
            }
            else
            {
                ImGui::TextWrapped("%s", assetData->name.c_str());
            }
    
            ImGui::NextColumn();
    
            ImGui::PopID();
        }

        if (m_currentFolder == ContentFolder::MATERIALS && ImGui::BeginPopupContextWindow(
            "ContentBrowserContextMenu",
            ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight
        ))
        {
            if (ImGui::MenuItem("Create Material"))
            {
                asset::OpaqueMaterialCreateInfo createInfo;
                createInfo.name = "NewMaterial";
                createInfo.roughness = 1.0;
                asset::AssetManager::create_material(createInfo);
            }


            ImGui::EndPopup();
        }
    }

    ImGui::Columns(1);

    ImGui::End();
}

void ContentBrowser::load_icons()
{
    TaskGroup taskGroup;

    TaskComposer::execute(taskGroup, [&](TaskExecutionInfo)
    {
        asset::TextureImportContext importContext;
        importContext.flags = asset::AssetFlag::TRANSIENT;
        importContext.projectDirectory = FileSystem::get_project_path();
        importContext.originalFilePath = "content/icons/folder.png";

        asset::TextureImportResult result;
        asset::AssetManager::import_texture(importContext, result);

        m_folderIcon = result.texture;
    });

    TaskComposer::execute(taskGroup, [&](TaskExecutionInfo)
    {
        asset::TextureImportContext importContext;
        importContext.flags = asset::AssetFlag::TRANSIENT;
        importContext.projectDirectory = FileSystem::get_project_path();
        importContext.originalFilePath = "content/icons/model.png";

        asset::TextureImportResult result;
        asset::AssetManager::import_texture(importContext, result);

        m_iconByAssetType[asset::Type::MODEL] = result.texture;
    });

    TaskComposer::execute(taskGroup, [&](TaskExecutionInfo)
    {
        asset::TextureImportContext importContext;
        importContext.flags = asset::AssetFlag::TRANSIENT;
        importContext.projectDirectory = FileSystem::get_project_path();
        importContext.originalFilePath = "content/icons/texture.png";

        asset::TextureImportResult result;
        asset::AssetManager::import_texture(importContext, result);

        m_iconByAssetType[asset::Type::TEXTURE] = result.texture;
    });

    TaskComposer::execute(taskGroup, [&](TaskExecutionInfo)
    {
        asset::TextureImportContext importContext;
        importContext.flags = asset::AssetFlag::TRANSIENT;
        importContext.projectDirectory = FileSystem::get_project_path();
        importContext.originalFilePath = "content/icons/material.png";

        asset::TextureImportResult result;
        asset::AssetManager::import_texture(importContext, result);

        m_iconByAssetType[asset::Type::MATERIAL] = result.texture;
    });

    TaskComposer::wait(taskGroup);
}

}