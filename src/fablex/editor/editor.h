#pragma once

#include "docking_window.h"
#include "viewport_window.h"
#include "outliner_window.h"
#include "properties_window.h"
#include "toolbar.h"
#include "content_browser.h"
#include "window_ui.h"

#include "core/fwd.h"
#include "engine/fwd.h"

#include <memory>

struct ImFont;

namespace fe::editor
{

class Editor
{
public:
    Editor();
    ~Editor();

    void draw();

    void set_window(Window* window);
    void set_world(engine::World* world);
    void set_camera(engine::Entity* entity);

private:
    Window* m_window = nullptr;
    engine::World* m_world = nullptr;
    engine::Entity* m_camera = nullptr;

    std::unique_ptr<DockingWindow> m_dockingWindow = nullptr;
    std::unique_ptr<ViewportWindow> m_viewportWindow = nullptr;
    std::unique_ptr<OutlinerWindow> m_outlinerWindow = nullptr;
    std::unique_ptr<PropertiesWindow> m_propertiesWindow = nullptr;
    std::unique_ptr<ContentBrowser> m_contentBrowser = nullptr;
    std::unique_ptr<Toolbar> m_toolbar = nullptr;

    ImFont* m_inconsolataMedium = nullptr;;

    std::vector<std::unique_ptr<WindowUI>> m_extraWindows;

    void subscribe_to_events();
    void load_fonts();
};

}