#pragma once

#include "common.h"
#include "core/uuid.h"

namespace fe::editor
{

class Utils
{
public:
    static void draw_properties_ui(const PropertyArray& properties, Object* object);

private:
    static void draw_material_component(Object* materialComponentObj);
};

}