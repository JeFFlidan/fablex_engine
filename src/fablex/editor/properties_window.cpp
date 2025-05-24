#pragma once

#include "properties_window.h"
#include "utils.h"

#include "engine/entity/entity.h"
#include "engine/entity/component.h"

#include "imgui.h"

namespace fe::editor
{

void PropertiesWindow::draw(engine::Entity* selectedEntity)
{
    if (selectedEntity)
        m_selectedEntity = selectedEntity;

    if (!m_selectedEntity)
    {
        ImGui::Begin("Properties");
        ImGui::End();
        return;
    }

    ImGui::Begin("Properties");

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        Float3 position = m_selectedEntity->get_position();

        if (ImGui::DragFloat3("Position", &position.x, 0.3f))
            m_selectedEntity->set_position(position);

        Float3 rotation = m_selectedEntity->get_euler_rotation();
        Float3 prevRotation = rotation;

        if (ImGui::DragFloat3("Rotation", &rotation.x, 0.2f, -10000.0f, 10000.0f))
        {
            Float3 rotationDelta = rotation - prevRotation;
            m_selectedEntity->set_rotation(rotationDelta);
        }

        Float3 scale = m_selectedEntity->get_scale();
        if (ImGui::DragFloat3("Scale", &scale.x, 0.3f, 0.0001f, 100000.0f))
            m_selectedEntity->set_scale(scale);
    }

    std::vector<Property*> properties;
    const TypeInfo* baseComponentTypeInfo = engine::Component::get_static_type_info();

    for (engine::Component* component : m_selectedEntity->get_components())
    {
        const TypeInfo* componentTypeInfo = component->get_type_info();

        if (ImGui::CollapsingHeader(componentTypeInfo->get_str_name(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (auto typeInfo = componentTypeInfo; typeInfo != baseComponentTypeInfo; typeInfo = typeInfo->get_base_type_info())
            {
                for (Property* property : typeInfo->get_properties())
                {
                    if (property->has_attribute<EditAnywhere>())
                        properties.push_back(property);
                }
            }

            std::reverse(properties.begin(), properties.end());

            Utils::draw_properties_ui(properties, component);
        }

        properties.clear();
    }

    ImGui::End();
}

}