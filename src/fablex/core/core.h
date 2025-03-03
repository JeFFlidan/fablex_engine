#pragma once

#include <string>

namespace fe
{

class Core
{
public:
    static void init();
    static void cleanup();

private:
    static std::string get_root_path();
};

}