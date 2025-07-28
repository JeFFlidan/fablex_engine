#pragma once

#include "engine/entity/component.h"

namespace fe::engine
{

class ShaderEntityComponent : public Component
{
    FE_DECLARE_COMPONENT(ShaderEntityComponent);

public:
    virtual void fill_shader_data(ShaderEntity& outShaderEntity) const override { }
    virtual bool is_light_source() const { return false; }
};

}