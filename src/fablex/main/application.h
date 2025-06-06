#pragma once

#include "core/window.h"
#include "engine/engine.h"
#include "editor/editor.h"
#include "renderer/renderer.h"

namespace fe
{

class Application
{
public:
    Application();
    ~Application();

    void execute();

private:
    std::unique_ptr<Window> m_mainWindow;
    std::unique_ptr<renderer::Renderer> m_renderer = nullptr;
    std::unique_ptr<engine::Engine> m_engine = nullptr;
    std::unique_ptr<editor::Editor> m_editor = nullptr;

    renderer::RendererConfig m_renderConfig;

    void load_engine_config();  // temp
};

}