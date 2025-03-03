#include "core/core.h"
#include "task_composer.h"
#include "file_system/file_system.h"

#include <filesystem>

namespace fe
{

void Core::init()
{
    TaskComposer::init();
    FileSystem::init(get_root_path());

    FE_LOG(LogDefault, INFO, "Root path: {}", fe::FileSystem::get_root_path());
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