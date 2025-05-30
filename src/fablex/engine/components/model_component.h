#pragma once

#include "asset_manager/fwd.h"
#include "engine/entity/component.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::engine
{

class ModelComponent : public Component
{
    FE_DECLARE_OBJECT(ModelComponent);
    FE_DECLARE_PROPERTY_REGISTER(ModelComponent);

public:
    void set_model(asset::Model* model);
    void set_model_uuid(UUID uuid);
    
    UUID get_model_uuid() const;
    asset::Model* get_model() const;

    bool is_model_loaded() const;

    void fill_shader_instance_data(ShaderModelInstance& outModelInstance) const;

    virtual void serialize(Archive& archive) const override;
    virtual void deserialize(Archive& archive) override;

protected:
    UUID m_modelUUID = UUID::INVALID;
};

}