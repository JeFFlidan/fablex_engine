#include "viewport_window.h"
#include "core/input.h"
#include "renderer/imgui_interop_editor.h"

#include "engine/entity/entity.h"
#include "engine/components/editor_camera_component.h"

#include "imgui.h"
#include "ImGuizmo.h"

namespace fe::editor
{

void ViewportWindow::draw(const EntitySet& selectedEntities)
{
    ImGui::Begin("Viewport");

    ViewportState viewportState = Input::get_viewport_state();
    viewportState.isHovered = ImGui::IsWindowHovered();

    ImVec2 newExtent = ImGui::GetContentRegionAvail();
    if (newExtent.x != viewportState.width || newExtent.y != viewportState.height)
    {
        viewportState.width = newExtent.x;
        viewportState.height = newExtent.y;
    }

    ImGui::Image(
        renderer::VIEWPORT_IMAGE_DESCRIPTOR, 
        ImVec2(viewportState.width, viewportState.height)
    );

    ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    ImVec2 vMax = ImGui::GetWindowContentRegionMax();

    viewportState.min = Float2(vMin.x, vMin.y);
    viewportState.max = Float2(vMax.x, vMax.y);

    viewportState.min.x += ImGui::GetWindowPos().x;
    viewportState.min.y += ImGui::GetWindowPos().y;
    viewportState.max.x += ImGui::GetWindowPos().x;
    viewportState.max.y += ImGui::GetWindowPos().y;

    Input::set_viewport_state(viewportState);

    draw_gizmo(selectedEntities);

    ImGui::End();
}

void ViewportWindow::set_camera(engine::Entity* camera)
{
    FE_CHECK(camera);
    m_camera = camera;
}

void ViewportWindow::draw_gizmo(const EntitySet& selectedEntities)
{
    if (selectedEntities.empty() || !ImGui::IsWindowHovered())
        return;

    FE_CHECK(m_camera);

    if (ImGui::IsKeyPressed(ImGuiKey_W) && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_E))
    {
        m_gizmoType = ImGuizmo::OPERATION::ROTATE;
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_R))
    {
        m_gizmoType = ImGuizmo::OPERATION::SCALE;
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_Tab))
    {
        m_gizmoType = -1;
        return;
    }

    if (m_gizmoType == -1)
        return;

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

    float windowWidth = ImGui::GetWindowWidth();
    float windowHeight = ImGui::GetWindowHeight();

    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

    Vector avgPosition = Vector3::create(0);
    Vector avgScale = Vector3::create(0);
    Float4 avgQuat = (*selectedEntities.begin())->get_rotation();

    uint32 entityCount = selectedEntities.size();
    
    for (engine::Entity* entity : selectedEntities)
    {
        avgPosition += entity->get_position();
        avgScale += entity->get_scale();

        if (entity == (*selectedEntities.begin()))
            continue;

        Float4 q = entity->get_rotation();
        float d = Quat::dot(avgQuat.to_quat(), q.to_quat());
        
        if (d < 0.0f)
            q = Float4(-q.x, -q.y, -q.z, -q.w);

        avgQuat.x += q.x;
        avgQuat.y += q.y;
        avgQuat.z += q.z;
        avgQuat.w += q.w;
    }

    avgPosition /= entityCount;
    avgScale /= entityCount;

    float invCount = 1.0f / entityCount;
    avgQuat.x *= invCount;
    avgQuat.y *= invCount;
    avgQuat.z *= invCount;
    avgQuat.w *= invCount;
    avgQuat = avgQuat.to_quat().normalize();

    Matrix scaleMat = Matrix::scaling(avgScale);
    Matrix rotationMat = Matrix::rotation(avgQuat.to_quat());
    Matrix translationMat = Matrix::translation(avgPosition);

    Float4x4 gizmoMat = scaleMat * rotationMat * translationMat;
    Matrix origGizmoMat = gizmoMat;

    auto cameraComp = m_camera->get_component<engine::EditorCameraComponent>();

    ImGuizmo::Manipulate(
        &cameraComp->view._11, 
        &cameraComp->projection._11, 
        (ImGuizmo::OPERATION)m_gizmoType, 
        ImGuizmo::MODE::WORLD, 
        &gizmoMat._11
    );

    if (ImGuizmo::IsUsing())
    {
        Vector translation, scale;
        Quat rotation;

        Matrix deltaMat = Matrix(gizmoMat) * origGizmoMat.inverse();
        
        for (engine::Entity* entity : selectedEntities)
        {
            Matrix newMatrix = deltaMat * entity->get_world_transform();
            newMatrix.decompose(translation, scale, rotation);

            entity->set_position(translation);
            entity->set_scale(scale);
            entity->set_rotation(rotation);
        }
    }
}

}