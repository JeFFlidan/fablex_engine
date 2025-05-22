#include "application.h"
#include "core/core.h"
#include "core/file_system/file_system.h"
#include "core/file_system/archive_test.h"
#include "asset_manager/asset_manager_test.h"

FE_DEFINE_LOG_CATEGORY(LogApplication)

namespace fe
{

void run_tests()
{
    FE_LOG(LogDefault, INFO, "Starting tests");
    ArchiveTest::run();
    asset::AssetManagerTest::run();
    FE_LOG(LogDefault, INFO, "Archive test completed successfully");
    FE_LOG(LogDefault, INFO, "Completed all tests");
}

Application::Application()
{
    FE_LOG(LogApplication, INFO, "Starting application initialization.");

    Core::init();

    WindowCreateInfo windowCreateInfo;
    windowCreateInfo.windowTitle = "Fablex Engine";
    windowCreateInfo.width = 1920;
    windowCreateInfo.height = 1080;
    m_mainWindow = std::make_unique<Window>();
    m_mainWindow->init(windowCreateInfo);

    load_engine_config();

    m_engine = std::make_unique<engine::Engine>();
    m_engine->set_window(m_mainWindow.get());

    m_editor = std::make_unique<editor::Editor>(m_mainWindow.get());

    renderer::RendererInfo rendererInfo;
    rendererInfo.config = &m_renderConfig;
    rendererInfo.window = m_mainWindow.get();
    m_renderer = std::make_unique<renderer::Renderer>(rendererInfo);

    // m_engine->configure_test_scene();
    m_engine->configure_sponza();

    FE_LOG(LogApplication, INFO, "Application initialization completed.");
}

Application::~Application()
{
    m_renderer.reset();
    TypeManager::cleanup();
    Core::cleanup();
}

void Application::execute()
{
    while (true)
    {
        if (!m_mainWindow->process_message())
        {
            FE_LOG(LogApplication, INFO, "Finish application execution.");
            break;
        }

        Core::update();
        m_renderer->predraw();
        m_editor->draw();
        m_engine->update();
        EventManager::dispatch_events();
        m_renderer->draw();
    }

    m_mainWindow->close();
}

void Application::load_engine_config()
{
    std::string engineConfigJsonStr;
    FileSystem::read(FileSystem::get_absolute_path("configs/engine.json"), engineConfigJsonStr);
    nlohmann::json engineConfigJson = nlohmann::json::parse(engineConfigJsonStr);
    m_renderConfig.init(engineConfigJson);
}

}