#pragma once

#include "core/window.h"
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

    renderer::RendererConfig m_renderConfig;

    void load_engine_config();  // temp
};

}