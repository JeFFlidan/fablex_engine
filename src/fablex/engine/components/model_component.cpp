#include "model_component.h"
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

}