#pragma once

#include "shader_entity_component.h"
#include "shaders/interops/shader_interop_entity.h"

namespace fe::engine
{

class LightComponent : public ShaderEntityComponent
{
    FE_DECLARE_COMPONENT(LightComponent);

public:
    Float4 color = Float4(1.0f, 1.0f, 1.0f, 1.0f);
    float intensity = 1.0;

    virtual void fill_shader_data(ShaderEntity& outShaderEntity) const override;
    
    virtual bool is_light_source() const override { return true; }

    virtual void serialize(Archive& archive) const override;
    virtual void deserialize(Archive& archive) override;
};

class DirectionalLightComponent : public LightComponent
{
    FE_DECLARE_COMPONENT(DirectionalLightComponent);

public:
    virtual void fill_shader_data(ShaderEntity& outShaderEntity) const override;
};

class PointLightComponent : public LightComponent
{
    FE_DECLARE_COMPONENT(PointLightComponent);

public:
    float attenuationRadius = 32.0f;

    virtual void fill_shader_data(ShaderEntity& outShaderEntity) const override;

    virtual void serialize(Archive& archive) const override;
    virtual void deserialize(Archive& archive) override;
};

}