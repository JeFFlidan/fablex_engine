#include "utils.h"

#include "engine/events.h"
#include "engine/entity/entity.h"
#include "engine/entities/model_entity.h"
#include "engine/components/events.h"
#include "engine/components/material_component.h"
#include "engine/components/model_component.h"

#include "core/object.h"
#include "core/file_system/file_system.h"

#include "asset_manager/asset_registry.h"
#include "asset_manager/asset_manager.h"
#include "asset_manager/json_serialization.h"
#include "asset_manager/model/model.h"
#include "asset_manager/events.h"

#include "imgui.h"

namespace fe::editor
{

void Utils::draw_properties_ui(const PropertyArray& properties, Object* object)
{
    for (Property* property : properties)
    {
        float minValue = 0.0f;
        float maxValue = 0.0f;
        float speed = 1.0f;

        if (const ClampMin* clampMin = property->get_attribute<ClampMin>())
            minValue = clampMin->min;

        if (const ClampMax* clampMax = property->get_attribute<ClampMax>())
            maxValue = clampMax->max;

        if (const SliderSpeed* sliderSpeed = property->get_attribute<SliderSpeed>())
            speed = sliderSpeed->speed;

        switch (property->get_type())
        {
            case PropertyType::BOOL:
            {
                bool value = property->get_value<bool>(object);
                ImGui::Checkbox(property->get_name().c_str(), &value);
                property->set_value(object, value);
                break;
            }
            case PropertyType::INTEGER:
            {
                int32 value = property->get_value<int32>(object);
                ImGui::SliderInt(property->get_name().c_str(), &value, (int)minValue, (int)maxValue);
                property->set_value(object, value);
                break;
            }
            case PropertyType::UUID:
            {
                draw_model_component(object);
                break;
            }
            case PropertyType::FLOAT:
            {
                float value = property->get_value<float>(object);
                ImGui::DragFloat(property->get_name().c_str(), &value, speed, minValue, maxValue);
                property->set_value(object, value);
                break;
            }
            case PropertyType::FLOAT2:
            {
                Float2 value = property->get_value<Float2>(object);
                ImGui::DragFloat2(property->get_name().c_str(), &value.x, speed, minValue, maxValue);
                property->set_value(object, value);
                break;
            }
            case PropertyType::FLOAT3:
            {
                Float3 value = property->get_value<Float3>(object);
                ImGui::DragFloat3(property->get_name().c_str(), &value.x, speed, minValue, maxValue);
                property->set_value(object, value);
                break;
            }
            case PropertyType::FLOAT4:
            {
                Float4 value = property->get_value<Float4>(object);

                if (property->get_attribute<Color>())
                    ImGui::ColorEdit4(property->get_name().c_str(), &value.x);
                else
                    ImGui::DragFloat4(property->get_name().c_str(), &value.x, speed, minValue, maxValue);

                property->set_value(object, value);
                break;
            }
            case PropertyType::FLOAT3X4:
            {
                FE_LOG(LogEditor, ERROR, "Float3x4 property UI is not implemented.");
                break;
            }
            case PropertyType::FLOAT4X4:
            {
                FE_LOG(LogEditor, ERROR, "Float4x4 property UI is not implemented.");
                break;
            }
            case PropertyType::QUAT:
            {
                FE_LOG(LogEditor, ERROR, "Quat property UI is not implemented.");
                break;
            }
            case PropertyType::STRING:
            {
                FE_LOG(LogEditor, ERROR, "String property UI is not implemented.");
                break;
            }
            case PropertyType::ARRAY:
            {
                // FE_LOG(LogDefault, INFO, "DRAW ARRAY");
                draw_material_component(object);
                break;
            }
        }
    }
}

void Utils::send_save_request()
{
    EventManager::enqueue_event(engine::ProjectSavingRequest());
}

void Utils::import_files(const std::string& currProjectDir)
{
    std::vector<std::string> paths;
    FileSystem::open_files_dialog(SUPPORTED_FILES, paths);

    for (const std::string& path : paths)
    {
        if (is_model_file(path))
        {
            asset::ModelImportContext importContext;
            importContext.generateMaterials = true;
            importContext.projectDirectory = currProjectDir;
            importContext.originalFilePath = path;
            EventManager::enqueue_event(asset::AssetImportRequestEvent(importContext));
        }
        if (is_texture_file(path))
        {
            asset::TextureImportContext importContext;
            importContext.originalFilePath = path;
            importContext.projectDirectory = currProjectDir;
            EventManager::enqueue_event(asset::AssetImportRequestEvent(importContext));
        }
    }
}

void Utils::setup_dark_theme()
{
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg]                = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
	colors[ImGuiCol_Border]                 = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
	colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
	colors[ImGuiCol_FrameBg]                = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_TitleBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_CheckMark]              = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_Button]                 = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_ButtonHovered]          = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_ButtonActive]           = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_Header]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
	colors[ImGuiCol_Separator]              = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_Tab]                    = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabHovered]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
	colors[ImGuiCol_TabUnfocused]           = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	//colors[ImGuiCol_DockingPreview]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	//colors[ImGuiCol_DockingEmptyBg]         = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram]          = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderLight]       = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_DragDropTarget]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_NavHighlight]           = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);
	
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding                     = ImVec2(8.00f, 8.00f);
	style.FramePadding                      = ImVec2(5.00f, 2.00f);
	style.CellPadding                       = ImVec2(6.00f, 6.00f);
	style.ItemSpacing                       = ImVec2(6.00f, 6.00f);
	style.ItemInnerSpacing                  = ImVec2(6.00f, 6.00f);
	style.TouchExtraPadding                 = ImVec2(0.00f, 0.00f);
	style.IndentSpacing                     = 25;
	style.ScrollbarSize                     = 15;
	style.GrabMinSize                       = 10;
	style.WindowBorderSize                  = 1;
	style.ChildBorderSize                   = 1;
	style.PopupBorderSize                   = 1;
	style.FrameBorderSize                   = 1;
	style.TabBorderSize                     = 1;
	style.WindowRounding                    = 7;
	style.ChildRounding                     = 4;
	style.FrameRounding                     = 3;
	style.PopupRounding                     = 4;
	style.ScrollbarRounding                 = 9;
	style.GrabRounding                      = 3;
	style.LogSliderDeadzone                 = 4;
	style.TabRounding                       = 4;
}

// Must not do this, but for now have no ideas how to draw this component in another way
void Utils::draw_material_component(Object* materialComponentObj)
{
    if (!materialComponentObj->is_a<engine::MaterialComponent>())
        return;

    auto materialComponent = reinterpret_cast<engine::MaterialComponent*>(materialComponentObj);
    FE_CHECK(materialComponent);
    FE_CHECK(materialComponent->get_entity());

    auto modelComponent = materialComponent->get_entity()->get_component<engine::ModelComponent>();
    FE_CHECK(modelComponent);

    auto model = modelComponent->get_model();
    FE_CHECK(model);

    const auto& allMaterials = asset::AssetRegistry::get_assets_data_by_type(asset::Type::MATERIAL);

    for (uint32 i = 0; i != model->material_slots().size(); ++i)
    {
        const asset::MaterialSlot& materialSlot = model->material_slots()[i];
        const UUID selectedMaterialUUID = materialComponent->material_uuids()[i];

        const asset::AssetData* selectedMaterialData = asset::AssetRegistry::get_asset_data_by_uuid(selectedMaterialUUID);
        const asset::AssetData* newSelectedMaterialData = selectedMaterialData;

        if (ImGui::BeginCombo(materialSlot.name.c_str(), selectedMaterialData->name.c_str()))
        {
            for (const asset::AssetData* matData : allMaterials)
            {
                if (ImGui::Selectable(matData->name.c_str(), matData == selectedMaterialData))
                {
                    newSelectedMaterialData = matData;
                    EventManager::enqueue_event(engine::MaterialUpdatedEvent(materialComponent));
                }

                if (matData == selectedMaterialData)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();

            if (!materialComponent->set_material(newSelectedMaterialData->uuid, i))
                FE_LOG(LogEditor, ERROR, "Material index {} is invalid.", i);
        }
    }
}

void Utils::draw_model_component(Object* modelComponentObj)
{
    if (!modelComponentObj->is_a<engine::ModelComponent>())
        return;

    auto modelComponent = static_cast<engine::ModelComponent*>(modelComponentObj);
    FE_CHECK(modelComponent);

    const auto& allModels = asset::AssetRegistry::get_assets_data_by_type(asset::Type::MODEL);
    auto prevAssetData = asset::AssetRegistry::get_asset_data_by_uuid(modelComponent->get_model_uuid());
    auto selectedAssetData = prevAssetData;

    if (ImGui::BeginCombo("Model", modelComponent->get_model()->get_name().c_str()))
    {
        for (auto assetData : allModels)
        {
            if (ImGui::Selectable(assetData->name.c_str(), assetData == prevAssetData))
                selectedAssetData = assetData;

            if (selectedAssetData == prevAssetData)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    if (selectedAssetData == prevAssetData)
        return;

    asset::Model* oldModel = asset::AssetManager::get_model(prevAssetData->uuid);
    asset::Model* currModel = asset::AssetManager::get_model(selectedAssetData->uuid);

    if (modelComponent->get_entity()->is_a<engine::ModelEntity>())
    {
        engine::ModelEntity* entity = static_cast<engine::ModelEntity*>(modelComponent->get_entity());
        entity->set_model(currModel);
        EventManager::enqueue_event(engine::ModelComponentUpdatedEvent(entity, oldModel, currModel));
    }
    else
    {
        auto matComponent = modelComponent->get_entity()->get_component<engine::MaterialComponent>();
        modelComponent->set_model(currModel);
        matComponent->init(currModel);

        EventManager::enqueue_event(engine::ModelComponentUpdatedEvent(modelComponent->get_entity(), oldModel, currModel));
    }
}

bool Utils::is_model_file(const std::string& name)
{
    static std::unordered_set<std::string> extensions = {
        "gltf",
        "glb"
    };

    return extensions.contains(FileSystem::get_file_extension(name));
}

bool Utils::is_texture_file(const std::string& name)
{
    static std::unordered_set<std::string> extensions = {
        "png",
        "jpg",
        "tiff",
        "dds",
        "tga",
        "hdr"
    };

    return extensions.contains(FileSystem::get_file_extension(name));
}

}