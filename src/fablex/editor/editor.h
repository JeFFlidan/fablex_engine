#pragma once

#include "docking_window.h"
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

    void set_world(engine::World* world);
    void set_window(Window* window);

private:
    Window* m_window = nullptr;
    engine::World* m_world = nullptr;

    std::unique_ptr<DockingWindow> m_dockingWindow = nullptr;
};

}