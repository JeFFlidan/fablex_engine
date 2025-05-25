#pragma once

#include "common.h"
#include "core/uuid.h"

namespace fe::editor
{

class Utils
{
public:
    static void draw_properties_ui(const PropertyArray& properties, Object* object);
    static void send_save_request();
    static void import_files(const std::string& currProjectDir);
    static void setup_dark_theme();

private:
    static void draw_material_component(Object* materialComponentObj);
    static void draw_model_component(Object* modelComponentObj);

    static bool is_model_file(const std::string& name);
    static bool is_texture_file(const std::string& name);
};

}