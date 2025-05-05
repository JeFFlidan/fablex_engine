#pragma once

#include "engine/entity/component.h"

namespace fe::engine
{

class ShaderEntityComponent : public Component
{
    FE_DECLARE_OBJECT(ShaderEntityComponent);
    FE_DECLARE_PROPERTY_REGISTER(ShaderEntityComponent);

public:
    virtual void fill_shader_data(ShaderEntity& outShaderEntity) const { }
    virtual bool is_light_source() const { return false; }
};

}