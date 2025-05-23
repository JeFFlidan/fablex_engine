#include "viewport_window.h"
#include "core/input.h"
#include "renderer/imgui_interop_editor.h"

#include "imgui.h"

namespace fe::editor
{

void ViewportWindow::draw(const EntitySet& entitySet) const
{
    ImGui::Begin("Viewport");

    ViewportState viewportState = Input::get_viewport_state();

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

    ImGui::End();
}

void ViewportWindow::set_camera(engine::Entity* camera)
{
    FE_CHECK(camera);
    m_camera = camera;
}

}