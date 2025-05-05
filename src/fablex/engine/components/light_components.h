#pragma once

#include "shader_entity_component.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::engine
{

class LightComponent : public ShaderEntityComponent
{
    FE_DECLARE_OBJECT(LightComponent);
    FE_DECLARE_PROPERTY_REGISTER(LightComponent);

public:
    Float4 color = Float4(1.0f, 1.0f, 1.0f, 1.0f);
    float intensity = 1.0;

    virtual void fill_shader_data(ShaderEntity& outShaderEntity) const override;
    
    virtual bool is_light_source() const override { return true; }
};

class DirectionalLightComponent : public LightComponent
{
    FE_DECLARE_OBJECT(DirectionalLightComponent);
    FE_DECLARE_PROPERTY_REGISTER(DirectionalLightComponent);

public:
    virtual void fill_shader_data(ShaderEntity& outShaderEntity) const override;
};

}