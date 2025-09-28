#pragma once

#include "core/types.h"

namespace fe::renderer
{

class GPUResourceCounters
{
public:
    static void reset();

    static uint32 next_light_index();
    static uint32 light_count();
    static uint32 light_offset();

    static uint32 shader_entity_count();

    static uint32 next_model_index();
    static uint32 model_count();

    static uint32 model_instance_count();

    static void increase_mesh_instance_count(uint32 meshCount);
    static uint32 mesh_instance_count();

    static uint32 next_material_index();
    static uint32 material_count();

private:
    inline static uint32 s_lightOffset = 0;
    inline static uint32 s_lightIndex = 0;

    inline static uint32 s_modelIndex = 0;
    inline static uint32 s_materialIndex = 0;
    inline static uint32 s_meshInstanceCount = 0;
};

}