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

    void save_world();

    void set_window(Window* window) { m_window = window; }
    World* get_world() const { return m_world.get(); }

    void configure_test_scene();
    void configure_sponza();

private:
    std::unique_ptr<World> m_world;
    Window* m_window;

    void create_default_material();
    void create_camera();
    void create_sun();

    bool load_project(const std::string& projectPath);

};

}