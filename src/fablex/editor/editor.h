#pragma once

#include "docking_window.h"
#include "viewport_window.h"
#include "outliner_window.h"

#include "core/fwd.h"
#include "engine/fwd.h"

#include <memory>

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
};

}