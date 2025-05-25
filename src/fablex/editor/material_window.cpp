#include "material_window.h"
#include "asset_manager/material/material.h"
#include "asset_manager/material/opaque_material_settings.h"

#include "asset_manager/asset_registry.h"
#include "asset_manager/asset_manager.h"

#include "imgui.h"

namespace fe::editor
{

MaterialWindow::MaterialWindow(asset::Material* material)
    : m_material(material)
{

}

bool MaterialWindow::draw()
{
    bool isOpen = true;

    std::string windowName = "Material " + m_material->get_name();

    if (ImGui::Begin(windowName.c_str(), &isOpen))
    {
        auto settings = m_material->material_settings<asset::OpaqueMaterialSettings>();

        Float4 color = settings->base_color(); 
        ImGui::ColorEdit4("Base Color", &color.x);
        settings->set_base_color(color);

        float roughness = settings->roughness();
        ImGui::SliderFloat("Roughness", &roughness, 0.0f, 10.0);
        settings->set_roughness(roughness);

        float metallic = settings->metallic();
        ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f);
        settings->set_metallic(metallic);

        UUID currTexture = settings->base_color_texture_uuid();
        UUID newTexture = draw_texture_combo_box(currTexture, "Base Color Texture");
        settings->set_base_color(newTexture);

        currTexture = settings->normal_texture_uuid();
        newTexture = draw_texture_combo_box(currTexture, "Normal Texture");
        settings->set_normal(newTexture);

        currTexture = settings->roughness_texture_uuid();
        newTexture = draw_texture_combo_box(currTexture, "Roughness Texture");
        settings->set_roughness(newTexture);

        currTexture = settings->metallic_texture_uuid();
        newTexture = draw_texture_combo_box(currTexture, "Metallic Texture");
        settings->set_metallic(newTexture);

        currTexture = settings->ambient_occlusion_texture_uuid();
        newTexture = draw_texture_combo_box(currTexture, "AO Texture");
        settings->set_ambient_occlusion(newTexture);

        currTexture = settings->arm_texture_uuid();
        newTexture = draw_texture_combo_box(currTexture, "ARM Texture");
        settings->set_arm(newTexture);
    }

    ImGui::End();

    return isOpen;
}

UUID MaterialWindow::draw_texture_combo_box(UUID selectedTextureUUID, const std::string& label)
{
    const asset::AssetData* selectedTexture = nullptr;
    if (selectedTextureUUID != UUID::INVALID)
        selectedTexture = asset::AssetRegistry::get_asset_data_by_uuid(selectedTextureUUID);

    auto allTextures = asset::AssetRegistry::get_assets_data_by_type(asset::Type::TEXTURE);
    UUID newSelectedTextureUUID = selectedTextureUUID;
    
    std::string previewValue = selectedTexture ? selectedTexture->name : "None";

    if (ImGui::BeginCombo(label.c_str(), previewValue.c_str()))
    {
        if (ImGui::Selectable("None", selectedTextureUUID == UUID::INVALID))
            newSelectedTextureUUID = UUID::INVALID;

        if (selectedTextureUUID == UUID::INVALID)
            ImGui::SetItemDefaultFocus();
        
        for (auto texture : allTextures)
        {
            if (ImGui::Selectable(texture->name.c_str(), texture == selectedTexture))
            {
                newSelectedTextureUUID = texture->uuid;
            }

            if (selectedTexture == texture)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    return newSelectedTextureUUID;
}

}