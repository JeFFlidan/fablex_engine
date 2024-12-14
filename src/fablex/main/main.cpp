#include <iostream>
#include "core/math.h"

int main(int argc, char* argv[])
{
    std::cout << "Hello, World!" << std::endl;

    fe::math::Float2 test(0.2, 0.3);
    std::cout << test.length() << std::endl;

    return 0;
}
