#include "gpu_model_instance.h"
#include "scene_manager.h"
#include "engine/entity/entity.h"
#include "engine/components/model_component.h"
#include "engine/components/material_component.h"
#include "asset_manager/model/model.h"
#include "core/primitives/sphere.h"

namespace fe::renderer
{

GPUModelInstance::GPUModelInstance(engine::Entity* entity) : m_entity(entity)
{
    m_modelComponent = m_entity->get_component<engine::ModelComponent>();
    m_materialComponent = m_entity->get_component<engine::MaterialComponent>();
}

void GPUModelInstance::fill_shader_model_instance(SceneManager* sceneManager, ShaderModelInstance& outModelInstance) const
{
    asset::Model* modelAsset = m_modelComponent->get_model();
    UUID materialUUID = m_materialComponent->get_material_uuid();
    
    const AABB& aabb = modelAsset->aabb(); 

    Sphere sphereBounds(aabb);
    outModelInstance.sphereBounds.center = sphereBounds.center;
    outModelInstance.sphereBounds.radius = sphereBounds.radius;
    outModelInstance.geometryOffset = sceneManager->resource_index(modelAsset->get_uuid());
    outModelInstance.materialIndex = sceneManager->resource_index(materialUUID);

    Matrix remapMat = modelAsset->aabb().get_unorm_remap_matrix();
    Matrix transformMat = m_entity->get_world_transform();

    outModelInstance.scale = m_entity->get_scale();
    outModelInstance.transform.set_transfrom(remapMat * transformMat);
    outModelInstance.rawTransform.set_transfrom(m_entity->get_world_transform());
    outModelInstance.transformInverseTranspose.set_transfrom(transformMat.transpose().inverse());
}

}