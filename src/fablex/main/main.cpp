#include <iostream>
#include "core/flags_operations.h"
#include "core/math.h"
#include "core/window.h"
#include "rhi/resources.h"
#include "rhi/rhi.h"
#include "rhi/vulkan/vulkan_rhi.h"
#include "core/pool_allocator.h"
#include "core/logger.h"
#include "renderer/shader_manager.h"
#include "core/name.h"

struct Test
{
    int a;
    int b;
    int c;
    int d;
};

int main(int argc, char* argv[])
{
    fe::rhi::vulkan::fill_function_table();

    std::cout << "Hello, World!" << std::endl;

    fe::math::Float2 test(0.2, 0.3);
    std::cout << test.length() << std::endl;

    fe::ThreadSafePoolAllocator<Test, 32> allocator;
    std::vector<Test*> objects;
    objects.reserve(32 * 2);
    for (int i = 0; i < 32 * 2; ++i)
    {
        objects.push_back(allocator.allocate(i, i, i, i));
    }
    std::cout << objects.size() << std::endl;
    for (auto& obj : objects)
    {
        std::cout << obj->a << " ";
    }

    std::cout << std::endl;

    allocator.free(objects.front());
    Test* obj = allocator.allocate(64, 312, 31, 52);
    std::cout << obj->a << " " << obj->b << " " << obj->c << " " << obj->d << std::endl;

    Test* obj2 = allocator.allocate(64, 312, 31, 52);
    std::cout << obj2->a << " " << obj2->b << " " << obj2->c << " " << obj2->d << std::endl;

    fe::rhi::ResourceFlags flags = fe::rhi::ResourceFlags::CUBE_TEXTURE | fe::rhi::ResourceFlags::RAW_BUFFER;
    if (has_flag(flags, fe::rhi::ResourceFlags::RAW_BUFFER))
    {
        std::cout << "has flag" << std::endl;
    }

    FE_LOG(LogDefault, INFO, "Hello World");
    FE_LOG(LogDefault, SUCCESS, "Success Hello World!!!");
    FE_LOG(LogDefault, WARNING, "Warning Hello World!!!");
    FE_LOG(LogDefault, ERROR, "Warning Hello World!!!");
    
    fe::Window window;
    window.init(fe::WindowCreateInfo());

    fe::rhi::RHIInitInfo initInfo;
    initInfo.validationMode = fe::rhi::ValidationMode::ENABLED;
    initInfo.gpuPreference = fe::rhi::GPUPreference::DISCRETE;
    fe::rhi::init(&initInfo);
    fe::renderer::ShaderManager::init();

    fe::Name name("forward_output");
    std::cout << name.to_string() << std::endl;

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
