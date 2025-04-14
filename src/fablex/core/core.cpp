#include "core.h"
#include "task_composer.h"
#include "file_system/file_system.h"
#include "timer.h"

#include <filesystem>

FE_DEFINE_LOG_CATEGORY(LogCore)

namespace fe
{

void Core::init()
{
    FE_LOG(LogCore, INFO, "Starting core systems initialization.");

    TaskComposer::init();
    FileSystem::init(get_root_path());
    Timer::init();

    FE_LOG(LogCore, INFO, "Core systems initialization completed");
    FE_LOG(LogCore, INFO, "Root path: {}", fe::FileSystem::get_root_path());
}

void Core::update()
{
    Timer::update();
}

void Core::cleanup()
{
    TaskComposer::cleanup();
}

std::string Core::get_root_path()
{
    std::filesystem::path binPath = std::filesystem::current_path();

    while (!binPath.empty() && binPath.filename() != "fablex_engine")
        binPath = binPath.parent_path();

    return binPath.string();
}

}