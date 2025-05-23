#include "editor.h"
#include "core/macro.h"
#include "core/window.h"
#include "engine/entity/world.h"

#include "imgui.h"

#ifdef WIN32
#include "imgui_impl_win32.h"
#endif // WIN32

namespace fe::editor
{

Editor::Editor()
{
    ImGui::CreateContext();

    m_dockingWindow = std::make_unique<DockingWindow>();
    m_viewportWindow = std::make_unique<ViewportWindow>();
}

Editor::~Editor()
{
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void Editor::draw()
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = static_cast<float>(m_window->get_info().width);
    io.DisplaySize.y = static_cast<float>(m_window->get_info().height);

    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    m_dockingWindow->draw();
    m_viewportWindow->draw();

    ImGui::Begin("Hello, ImGui!");
    ImGui::Text("Simple Window");
    ImGui::End();

    ImGui::Render();
}

void Editor::set_window(Window* window)
{
#ifdef WIN32
    if (m_window)
        ImGui_ImplWin32_Shutdown();

    m_window = window;
    ImGui_ImplWin32_Init(m_window->get_info().win32Window.hWnd);
#endif // WIN32
}

void Editor::set_world(engine::World* world)
{
    FE_CHECK(world);
    m_world = world;
}

void Editor::set_camera(engine::Entity* camera)
{
    FE_CHECK(camera);
    m_camera = camera;
}

}