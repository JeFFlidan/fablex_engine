#pragma once

#include "entity/world.h"
#include "core/window.h"
#include <memory>

namespace fe::engine
{

class Engine
{
public:
    Engine();

    void update();

    void set_window(Window* window) { m_window = window; }

    void configure_test_scene();
    void configure_sponza();

private:
    std::unique_ptr<World> m_world;
    Window* m_window;

};

}