#pragma once

#include "engine/entity/entity.h"
#include "asset_manager/fwd.h"
#include "engine/components/fwd.h"

namespace fe::engine
{

class ModelEntity : public Entity
{
    FE_DECLARE_OBJECT(ModelEntity);

public:
    ModelEntity();

    virtual void init() override;

    void set_model(asset::Model* model);
    ModelComponent* get_model_component() const { return m_modelComponent; }
    MaterialComponent* get_material_component() const { return m_materialComponent; }

private:
    ModelComponent* m_modelComponent = nullptr;
    MaterialComponent* m_materialComponent = nullptr;
};

}