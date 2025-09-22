#include "gpu_material.h"
#include "scene_manager.h"

namespace fe::renderer
{

GPUMaterial::GPUMaterial(asset::Material* material) : GPUResource(material)
{

}

void GPUMaterial::build(SceneManager* sceneManager, const CommandRecorder& cmdRecorder)
{
    
}

void GPUMaterial::fill_shader_data(const SceneManager* sceneManager)
{
    const MaterialBuffers& materialBuffers = sceneManager->material_buffers();
    ShaderMaterial& shaderMaterial = materialBuffers[m_indexInBuffer];
    shaderMaterial.init();
    m_asset->fill_shader_material(shaderMaterial);
}

}
