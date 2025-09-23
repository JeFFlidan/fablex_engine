#pragma once

#include "gpu_resource.h"
#include "engine/components/material_component.h"
#include "asset_manager/material/material.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

class SceneManager;
class CommandRecorder;

class GPUMaterial : public GPUResource<asset::Material>
{
public:
    GPUMaterial(asset::Material* material);

    void build(SceneManager* sceneManager);
    bool upload_to_gpu(const SceneManager* sceneManager);
};

}