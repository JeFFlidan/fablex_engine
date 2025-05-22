#include "editor.h"
#include "core/logger.h"

#include "imgui.h"
#include "imgui_impl_win32.h"

namespace fe::editor
{

Editor::Editor(Window* window) : m_window(window)
{
    FE_CHECK(m_window);

    ImGui::CreateContext();

#ifdef WIN32
    ImGui_ImplWin32_Init(m_window->get_info().win32Window.hWnd);
#endif // WIN32
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

    ImGui::Begin("Hello, ImGui!");
    ImGui::Text("Simple Window");
    ImGui::End();

    ImGui::Render();
}

}