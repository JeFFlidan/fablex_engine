#include "outliner_window.h"
#include "engine/entity/world.h"
#include "engine/events.h"

#include "imgui.h"

namespace fe::editor
{

void OutlinerWindow::draw(engine::World* world)
{
    m_allDrawnEntities.clear();
    m_anyItemHovered = false;

    ImGui::Begin("Outliner");

    for (engine::Entity* entity : world->get_entities())
    {
        if (!entity->get_root())
            draw_node(entity);
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !m_anyItemHovered)
    {
        m_lastSelectedEntity = nullptr;
        m_renamedEntity = nullptr;
        m_selectedEntities.clear();
    }

    if (ImGui::BeginPopupContextWindow(
        "OutlinerContextMenu",
        ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight
    ))
    {
        if (ImGui::MenuItem("Create Model"))
            EventManager::enqueue_event(engine::ModelEntityCreationRequest());

        if (ImGui::MenuItem("Point Light"))
            EventManager::enqueue_event(engine::PointLightEntityCreationRequest());

        ImGui::EndPopup();
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete))
        remove_selected_entities();

    handle_shift_selection();

    ImGui::End();
}

void OutlinerWindow::draw_node(engine::Entity* entity)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    m_allDrawnEntities.push_back(entity);

    if (entity->get_children().empty())
        flags |= ImGuiTreeNodeFlags_Leaf;

    if (m_selectedEntities.contains(entity))
    {
        flags |= ImGuiTreeNodeFlags_Selected;
        select_children(entity);
    }

    bool isNodeOpened = false;

    ImGui::PushID(entity);

    if (m_renamedEntity == entity)
    {
        char buffer[128];
        strncpy_s(buffer, entity->get_name().c_str(), sizeof(buffer));
        ImGui::SetKeyboardFocusHere();

        if (ImGui::InputText("##rename", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) 
        {
            entity->set_name(buffer);
            m_renamedEntity = nullptr;
        }

        isNodeOpened = ImGui::TreeNodeEx("##hidden", flags, "");
    }
    else
    {
        isNodeOpened = ImGui::TreeNodeEx(entity->get_name().c_str(), flags);

        if (ImGui::IsItemClicked())
        {
            handle_click(entity);
            m_renamedEntity = nullptr;
        }
        
        if (ImGui::BeginPopupContextItem())
        {
            if (!m_selectedEntities.contains(entity))
                m_selectedEntities.clear();

            if (ImGui::MenuItem("Rename"))
                m_renamedEntity = entity;

            ImGui::Separator();

            if (ImGui::MenuItem("Remove"))
            {
                EventManager::enqueue_event(engine::EntityRemovalRequest(entity));
                remove_selected_entities();
            }

            ImGui::EndPopup();
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_F2) && entity == m_lastSelectedEntity)
    {
        m_renamedEntity = entity;
    }

    if (ImGui::IsItemHovered())
        m_anyItemHovered = true;

    if (isNodeOpened)
    {
        for (engine::Entity* child : entity->get_children())
            draw_node(child);
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void OutlinerWindow::handle_click(engine::Entity* entity)
{
    if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && m_lastSelectedEntity)
    {   
        m_selectedByShitfEntity = entity;
    }
    else if (ImGui::IsKeyDown(ImGuiKey::ImGuiMod_Ctrl))
    {
        if (m_selectedEntities.contains(entity))
        {
            deselect_entity(entity);
            m_lastSelectedEntity = nullptr;     // TEMP
        }
        else
        {
            m_lastSelectedEntity = entity;
            select_entity(entity);
            select_children(entity);
        }
    }
    else
    {
        m_selectedEntities.clear();
        m_lastSelectedEntity = entity;
        select_entity(entity);
        select_children(entity);
    }
}

void OutlinerWindow::handle_shift_selection()
{
    if (!m_selectedByShitfEntity || !m_lastSelectedEntity)
        return;

    auto it1 = std::find(m_allDrawnEntities.begin(), m_allDrawnEntities.end(), m_lastSelectedEntity);
    auto it2 = std::find(m_allDrawnEntities.begin(), m_allDrawnEntities.end(), m_selectedByShitfEntity);
    if (it1 != m_allDrawnEntities.end() && it2 != m_allDrawnEntities.end()) 
    {
        if (it1 > it2) std::swap(it1, it2);
        for (auto it = it1; it <= it2; ++it)
        {
            select_entity((*it));
            select_children((*it));
        }

        m_lastSelectedEntity = m_selectedByShitfEntity;
        m_selectedByShitfEntity = nullptr;
    }
}

void OutlinerWindow::select_children(engine::Entity* entity)
{
    for (engine::Entity* child : entity->get_children())
        select_entity(child);
}

void OutlinerWindow::deselect_children(engine::Entity* entity)
{
    for (engine::Entity* child : entity->get_children())
        deselect_entity(child);
}

void OutlinerWindow::select_entity(engine::Entity* entity)
{
    if (m_selectedEntities.contains(entity))
        return;

    m_selectedEntities.insert(entity);
}

void OutlinerWindow::deselect_entity(engine::Entity* entity)
{
    if (!m_selectedEntities.contains(entity))
        return;

    m_selectedEntities.erase(entity);
}

void OutlinerWindow::remove_selected_entities()
{
    for (engine::Entity* entity : m_selectedEntities)
        EventManager::enqueue_event(engine::EntityRemovalRequest(entity));

    m_selectedEntities.clear();
}

}