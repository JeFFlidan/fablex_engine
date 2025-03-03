#include "application.h"
#include "core/core.h"

namespace fe
{

void Application::init()
{
    Core::init();
}

void Application::cleanup()
{
    Core::cleanup();
}

}