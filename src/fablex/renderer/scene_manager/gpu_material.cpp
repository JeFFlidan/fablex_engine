#include "gpu_material.h"
#include "scene_manager.h"

namespace fe::renderer
{

GPUMaterial::GPUMaterial(asset::Material* material) : GPUResource(material)
{

}

void GPUMaterial::build(SceneManager* sceneManager)
{
    
}

bool GPUMaterial::upload_to_gpu(const SceneManager* sceneManager)
{
    if (m_refCount == 0)
        return false;

    const MaterialBuffers& materialBuffers = sceneManager->material_buffers();
    ShaderMaterial& shaderMaterial = materialBuffers[m_indexInBuffer];
    shaderMaterial.init();
    m_asset->fill_shader_material(shaderMaterial);

    return true;
}

}
