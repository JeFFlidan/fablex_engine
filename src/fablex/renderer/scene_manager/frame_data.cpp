#include "frame_data.h"
#include "scene_manager.h"
#include "gpu_resource_counters.h"

namespace fe::renderer
{

void FrameData::fill(const SceneManager* sceneManager)
{
    modelBufferIndex = sceneManager->model_buffers().descriptor();
    modelInstanceBufferIndex = sceneManager->model_instance_buffers().descriptor();
    meshInstanceBufferIndex = sceneManager->mesh_instance_buffers().descriptor();
    entityBufferIndex = sceneManager->shader_entity_buffers().descriptor();
    materialBufferIndex = sceneManager->material_buffers().descriptor();
    lightArrayCount = GPUResourceCounters::light_count();
    lightArrayOffset = GPUResourceCounters::light_offset();
}

}