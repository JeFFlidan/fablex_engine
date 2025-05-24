#include "model_component.h"
#include "engine/entity/entity.h"
#include "core/primitives/sphere.h"
#include "core/file_system/archive.h"
#include "asset_manager/asset_manager.h"

namespace fe::engine
{

FE_DEFINE_OBJECT(ModelComponent, Component);

FE_BEGIN_PROPERTY_REGISTER(ModelComponent)
{
    FE_REGISTER_PROPERTY(ModelComponent, m_modelUUID);
}
FE_END_PROPERTY_REGISTER(ModelComponent)

void ModelComponent::set_model(asset::Model* model)
{
    m_modelUUID = model->get_uuid();
}

void ModelComponent::set_model_uuid(UUID uuid)
{
    m_modelUUID = uuid;
}

UUID ModelComponent::get_model_uuid() const
{
    return m_modelUUID;
}

asset::Model* ModelComponent::get_model() const
{
    return asset::AssetManager::get_model(m_modelUUID);
}

bool ModelComponent::is_model_loaded() const
{
    return asset::AssetManager::is_asset_loaded(m_modelUUID);
}

void ModelComponent::fill_shader_instance_data(ShaderModelInstance& outModelInstance) const
{
    const AABB& aabb = get_model()->aabb(); 

    Sphere sphereBounds(aabb);
    outModelInstance.sphereBounds.center = sphereBounds.center;
    outModelInstance.sphereBounds.radius = sphereBounds.radius;

    Matrix remapMat = get_model()->aabb().get_unorm_remap_matrix();
    Matrix transformMat = m_entity->get_world_transform();

    outModelInstance.scale = m_entity->get_scale();
    outModelInstance.transform.set_transfrom(remapMat * transformMat);
    outModelInstance.rawTransform.set_transfrom(m_entity->get_world_transform());
    outModelInstance.transformInverseTranspose.set_transfrom(transformMat.transpose().inverse());
}

void ModelComponent::serialize(Archive& archive) const
{
    Component::serialize(archive);

    archive << m_modelUUID;
}

void ModelComponent::deserialize(Archive& archive)
{
    Component::deserialize(archive);

    archive >> m_modelUUID;
}

}