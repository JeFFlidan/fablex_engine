#include "gpu_material.h"
#include "scene_manager.h"
#include "asset_manager/material/material.h"

namespace fe::renderer
{

GPUMaterial::GPUMaterial(asset::Material* material) : m_material(material)
{

}

void GPUMaterial::build(SceneManager* sceneManager, const CommandRecorder& cmdRecorder)
{
    
}

void GPUMaterial::fill_shader_material(const SceneManager* sceneManager, ShaderMaterial& shaderMaterial) const
{
    m_material->fill_shader_material(shaderMaterial);
}

}
