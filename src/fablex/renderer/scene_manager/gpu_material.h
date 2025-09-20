#pragma once

#include "gpu_resource.h"
#include "frame_data_buffers.h"
#include "engine/components/material_component.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

class SceneManager;
class CommandRecorder;

class GPUMaterial : public GPUResource
{
public:
    GPUMaterial(asset::Material* material);

    void build(SceneManager* sceneManager, const CommandRecorder& cmdRecorder);
    void fill_shader_material(const SceneManager* sceneManager, ShaderMaterial& shaderMaterial) const;
    void fill_shader_data(const SceneManager* sceneManager);

    asset::Material* material_asset() const { return m_material; }

private:
    asset::Material* m_material;
};

}