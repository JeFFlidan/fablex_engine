#include "toolbar.h"
#include "utils.h"
#include "common.h"
#include "engine/events.h"
#include "core/file_system/file_system.h"

#include "imgui.h"

namespace fe::editor
{

void Toolbar::draw(const std::string& currProjectPath)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Project", "Ctrl+N"))
            {

            }
            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                Utils::send_save_request();
            }
            if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
            {
                
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Import", "Ctrl+I"))
            {
                Utils::import_files(currProjectPath);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

}