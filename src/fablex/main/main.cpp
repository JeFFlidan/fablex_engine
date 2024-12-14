#include <iostream>
#include "core/math.h"
#include "core/window.h"

int main(int argc, char* argv[])
{
    std::cout << "Hello, World!" << std::endl;

    fe::math::Float2 test(0.2, 0.3);
    std::cout << test.length() << std::endl;

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

    return 0;
}
