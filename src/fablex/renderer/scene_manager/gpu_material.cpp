#include "gpu_material.h"
#include "scene_manager.h"
#include "globals.h"
#include "asset_manager/material/opaque_material.h"

namespace fe::renderer
{

GPUMaterial::GPUMaterial(asset::Material* material) : m_material(material)
{

}

GPUMaterial::GPUMaterial(engine::MaterialComponent* materialComponent) : m_material(materialComponent->get_material())
{

}

void GPUMaterial::build(SceneManager* sceneManager, const CommandRecorder& cmdRecorder)
{
    switch (m_material->get_material_type())
    {
    case asset::MaterialType::OPAQUE:
    {
        asset::OpaqueMaterial* opaqueMaterial = static_cast<asset::OpaqueMaterial*>(m_material);
        
        if (opaqueMaterial->has_base_color_texture())
            sceneManager->load_texture_into_gpu(opaqueMaterial->base_color_texture_uuid());

        if (opaqueMaterial->has_normal_texture())
            sceneManager->load_texture_into_gpu(opaqueMaterial->normal_texture_uuid());

        if (opaqueMaterial->has_roughness_texture())
            sceneManager->load_texture_into_gpu(opaqueMaterial->roughness_texture_uuid());

        if (opaqueMaterial->has_metallic_texture())
            sceneManager->load_texture_into_gpu(opaqueMaterial->metallic_texture_uuid());

        if (opaqueMaterial->has_ambient_occlusion_texture())
            sceneManager->load_texture_into_gpu(opaqueMaterial->ambient_occlusion_texture_uuid());

        break;
    }
    default:
        FE_CHECK(0);
    }
}

void GPUMaterial::fill_shader_material(const SceneManager* sceneManager, ShaderMaterial& shaderMaterial) const
{
    auto setTextureSlot = [&](TextureSlot textureSlot, asset::Texture* texture)
    {
        shaderMaterial.textures[textureSlot].init();

        if (texture)
            shaderMaterial.textures[textureSlot].textureIndex = sceneManager->get_descriptor(texture);
    };

    shaderMaterial.set_sampler_index(sceneManager->get_sampler_descriptor(SAMPLER_LINEAR_REPEAT));

    switch (m_material->get_material_type())
    {
    case asset::MaterialType::OPAQUE:
    {
        asset::OpaqueMaterial* opaqueMaterial = static_cast<asset::OpaqueMaterial*>(m_material);

        shaderMaterial.set_base_color(opaqueMaterial->base_color());
        shaderMaterial.set_roughness(opaqueMaterial->roughness());
        shaderMaterial.set_metallic(opaqueMaterial->metallic());

        setTextureSlot(TEXTURE_SLOT_BASE_COLOR, opaqueMaterial->base_color_texture());
        setTextureSlot(TEXTURE_SLOT_NORMAL, opaqueMaterial->normal_texture());
        setTextureSlot(TEXTURE_SLOT_ROUGHNESS, opaqueMaterial->roughness_texture());
        setTextureSlot(TEXTURE_SLOT_METALLIC, opaqueMaterial->metallic_texture());
        setTextureSlot(TEXTURE_SLOT_AO, opaqueMaterial->ambient_occlusion_texture());

        break;
    }
    default:
        FE_CHECK(0);
    }
}

}
