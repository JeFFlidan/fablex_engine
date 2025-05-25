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
    World* get_world() const { return m_world.get(); }

    void configure_test_scene();
    void configure_sponza();

    void create_project(const std::string& projectName);
    void save_project();
    bool load_project(const std::string& projectPath);

private:
    std::unique_ptr<World> m_world;
    Window* m_window;

    void subscribe_to_events();

    void create_default_model();
    void create_default_material();
    void create_camera();
    void create_sun();

    void save_world();
};

}