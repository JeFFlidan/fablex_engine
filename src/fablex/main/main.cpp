#include "core/window.h"
#include "application.h"
#include <iostream>

int main(int argc, char* argv[])
{   
    fe::Application app;
    app.init();

    fe::Window window;
    window.init(fe::WindowCreateInfo());

    while (true)
    {
        if (!window.process_message())
        {
            std::cout << "finish" << std::endl;
            break;
        }
    }

    window.close();
    app.cleanup();

    return 0;
}
