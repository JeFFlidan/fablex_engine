#include "gpu_resource_counters.h"
#include "engine/components/light_components.h"
#include "engine/components/model_component.h"

namespace fe::renderer
{

void GPUResourceCounters::reset()
{
    s_lightOffset = 0;
    s_lightIndex = 0;
    s_modelIndex = 0;
    s_materialIndex = 0;
    s_meshInstanceCount = 0;
}

uint32 GPUResourceCounters::next_light_index()
{
    return s_lightOffset + s_lightIndex++;
}

uint32 GPUResourceCounters::light_count()
{
    return engine::LightComponent::count();
}

uint32 GPUResourceCounters::light_offset()
{
    return s_lightOffset;
}

uint32 GPUResourceCounters::shader_entity_count()
{
    return engine::ShaderEntityComponent::count();
}

uint32 GPUResourceCounters::next_model_index()
{
    return s_modelIndex++;
}

uint32 GPUResourceCounters::model_count()
{
    return s_modelIndex;
}

uint32 GPUResourceCounters::model_instance_count()
{
    return engine::ModelComponent::count();
}

void GPUResourceCounters::increase_mesh_instance_count(uint32 meshCount)
{
    s_meshInstanceCount += meshCount;
}

uint32 GPUResourceCounters::mesh_instance_count()
{
    return s_meshInstanceCount;
}

uint32 GPUResourceCounters::next_material_index()
{
    return s_materialIndex++;
}

uint32 GPUResourceCounters::material_count()
{
    return s_materialIndex;
}

}