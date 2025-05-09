#pragma once

#include "engine/fwd.h"

struct ShaderModelInstance;

namespace fe::renderer
{

class SceneManager;

class GPUModelInstance
{
public:
    GPUModelInstance(engine::Entity* entity);

    void fill_shader_model_instance(SceneManager* sceneManager, ShaderModelInstance& outModelInstance) const;

private:
    engine::Entity* m_entity = nullptr;
    engine::ModelComponent* m_modelComponent = nullptr;
    engine::MaterialComponent* m_materialComponent = nullptr;
};

}