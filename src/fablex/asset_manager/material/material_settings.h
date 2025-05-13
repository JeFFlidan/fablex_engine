#pragma once

#include "core/object.h"
#include "common.h"
#include <functional>
#include <memory>

struct ShaderMaterial;

namespace fe::asset
{

class Texture;

enum class MaterialType
{
    UNDEFINED,
    OPAQUE,
};

class MaterialSettings : public Object
{
    FE_DECLARE_OBJECT(MaterialSettings);

public:
    virtual void fill_shader_material(ShaderMaterial& outShaderMaterial) { };
    virtual MaterialType material_type() const { return MaterialType::UNDEFINED; };

protected:
    void set_texture(ShaderMaterial& outShaderMaterial, uint32 textureSlot, Texture* textureAsset) const;
};

using MaterialSettingsHandle = std::unique_ptr<MaterialSettings>;

struct MaterialCreateInfo : public CreateInfo
{
    using InitHandler = std::function<MaterialSettingsHandle()>;

    MaterialType type;
    InitHandler initHandler;
};

}