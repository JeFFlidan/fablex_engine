#pragma once

#include "engine/components/material_component.h"

namespace fe::renderer
{

class SceneManager;
class CommandRecorder;

class GPUMaterial
{
public:
    GPUMaterial(asset::Material* material);
    GPUMaterial(engine::MaterialComponent* materialComponent);

    void build(SceneManager* sceneManager, const CommandRecorder& cmdRecorder);
    void fill_shader_material(const SceneManager* sceneManager, ShaderMaterial& shaderMaterial) const;

    asset::Material* get_material() const { return m_material; }

private:
    asset::Material* m_material;
};

}