#pragma once

#include "gpu_resource.h"
#include "engine/fwd.h"

struct ShaderModelInstance;

namespace fe::renderer
{

class SceneManager;

class GPUModelInstance : public GPUResource
{
public:
    GPUModelInstance(engine::Entity* entity);

    void fill_shader_model_instance(ShaderModelInstance& outModelInstance) const;

    engine::Entity* entity() const { return m_entity; }
    engine::MaterialComponent* material_component() const { return m_materialComponent; }
    engine::ModelComponent* model_component() const { return m_modelComponent; }

    uint32 mesh_instance_range_begin_index() const { return m_meshInstanceRangeBeginIndex; }
    void mesh_instance_range_begin_index(uint32 index) { m_meshInstanceRangeBeginIndex = index; }

private:
    engine::Entity* m_entity = nullptr;
    engine::ModelComponent* m_modelComponent = nullptr;
    engine::MaterialComponent* m_materialComponent = nullptr;

    uint32 m_meshInstanceRangeBeginIndex = 0;
};

}