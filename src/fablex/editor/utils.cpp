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