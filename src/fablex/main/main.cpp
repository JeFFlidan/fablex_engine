#include "core/window.h"
#include "application.h"
#include "renderer/render_pass.h"
#include "core/logger.h"
#include <iostream>

int main(int argc, char* argv[])
{   
    fe::Application app;
    app.init();

    fe::renderer::RenderPass* renderPass = (fe::renderer::RenderPass*)fe::create_object("TestPass");
    FE_CHECK(renderPass);
    FE_LOG(LogDefault, INFO, "BEFORE EXECUTE {}", renderPass->get_type_info()->get_name());
    renderPass->execute(nullptr);
    FE_LOG(LogDefault, INFO, "AFTER EXECUTE");

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
