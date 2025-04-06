#include "asset_manager.h"
#include <unordered_set>

namespace fe::asset
{

const std::unordered_set<std::string> SUPPORTED_MODEL_FORMATS = {
    "gltf",
    "glb"
};

const std::unordered_set<std::string> SUPPORTED_TEXTURE_FORMATS = {
    "tga",
    "png",
    "tiff",
    "jpg"
};

Model* AssetManager::create_model(const ModelCreateInfo& createInfo)
{
    // TODO: Add model to asset table
    Model* model = allocate<Model>();
    model->m_name = createInfo.name;
    return model;
}

Texture* AssetManager::create_texture(const TextureCreateInfo& createInfo)
{
    // TODO: Add texture to asset table
    Texture* texture = allocate<Texture>();
    texture->m_name = createInfo.name;
    return texture;
}

Material* AssetManager::create_material(const MaterialCreateInfo& createInfo)
{
    // TODO: Add material to asset table
    Material* material = allocate<Material>();
    material->m_name = createInfo.name;
    return material;
}

bool AssetManager::is_model_format_supported(const std::string& extension)
{
    auto it = SUPPORTED_MODEL_FORMATS.find(extension);
    if (it == SUPPORTED_MODEL_FORMATS.end())
        return false;
    return true;
}

bool AssetManager::is_texture_format_supported(const std::string& extension)
{
    auto it = SUPPORTED_TEXTURE_FORMATS.find(extension);
    if (it == SUPPORTED_TEXTURE_FORMATS.end())
        return false;
    return true;
}

}