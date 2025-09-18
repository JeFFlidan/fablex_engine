#include "material_settings.h"
#include "renderer/accessor.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::asset
{

FE_DEFINE_OBJECT(MaterialSettings, Object);

void MaterialSettings::set_texture(
    ShaderMaterial& outShaderMaterial, 
    uint32 textureSlot, 
    Texture* textureAsset
) const
{
    outShaderMaterial.textures[textureSlot].init();
    if (textureAsset)
        outShaderMaterial.textures[textureSlot] = renderer::Accessor::get_descriptor(textureAsset);
}

}