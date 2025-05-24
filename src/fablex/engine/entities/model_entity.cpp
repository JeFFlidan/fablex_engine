#include "model_entity.h"
#include "engine/components/model_component.h"
#include "engine/components/material_component.h"

#include "asset_manager/asset_manager.h"

namespace fe::engine
{

FE_DEFINE_OBJECT(ModelEntity, Entity);

ModelEntity::ModelEntity()
{

}

void ModelEntity::init()
{
    m_modelComponent = create_component<ModelComponent>();
    m_modelComponent->set_model(asset::AssetManager::get_default_model());

    m_materialComponent = create_component<MaterialComponent>();
    m_materialComponent->init(m_modelComponent->get_model());
}

void ModelEntity::set_model(asset::Model* model)
{
    FE_CHECK(model);

    m_modelComponent->set_model(model);
    m_materialComponent->init(model);
}

}