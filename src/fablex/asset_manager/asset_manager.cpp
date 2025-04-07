#include "asset_manager.h"
#include "model/model_bridge.h"
#include "texture/texture_bridge.h"
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

void AssetManager::init()
{
    EventManager::subscribe<AssetImportRequestEvent<ModelImportContext>>(
        [](const AssetImportRequestEvent<ModelImportContext>& event)
        {
            ModelImportResult importResult;
            import_model(event.get_import_context(), importResult);
        }
    );

    EventManager::subscribe<AssetImportRequestEvent<TextureImportContext>>(
        [](const AssetImportRequestEvent<TextureImportContext>& event)
        {
            TextureImportResult importResult;
            import_texture(event.get_import_context(), importResult);
        }
    );
}

Model* AssetManager::create_model(const ModelCreateInfo& createInfo)
{
    Model* model = allocate<Model>();
    model->m_name = createInfo.name;
    model->make_dirty();
    return model;
}

Texture* AssetManager::create_texture(const TextureCreateInfo& createInfo)
{
    Texture* texture = allocate<Texture>();
    texture->m_name = createInfo.name;
    texture->make_dirty();
    return texture;
}

Material* AssetManager::create_material(const MaterialCreateInfo& createInfo)
{
    Material* material = allocate<Material>();
    material->m_name = createInfo.name;

    material->make_dirty();

    EventManager::enqueue_event(AssetCreatedEvent<Material>(material));
    
    return material;
}

bool AssetManager::import_model(const ModelImportContext& inImportContext, ModelImportResult& outImportResult)
{
    if (!ModelBridge::import(inImportContext, outImportResult))
        return false;

    for (Model* model : outImportResult.models)
    {
        configure_imported_asset(model, inImportContext);
        EventManager::enqueue_event(AssetImportedEvent<Model>(model));
    }

    for (Texture* texture : outImportResult.textures)
    {
        configure_imported_asset(texture, inImportContext);
        EventManager::enqueue_event(AssetImportedEvent<Texture>(texture));
    }

    return true;
}

bool AssetManager::import_texture(const TextureImportContext& inImportContext, TextureImportResult& outImportResult)
{
    if (!TextureBridge::import(inImportContext, outImportResult))
        return false;

    configure_imported_asset(outImportResult.texture, inImportContext);
    EventManager::enqueue_event(AssetImportedEvent<Texture>(outImportResult.texture));
    
    return true;
}

Model* AssetManager::get_model(UUID uuid)
{
    return get_asset<Model>(uuid);
}

Texture* AssetManager::get_texture(UUID uuid)
{
    return get_asset<Texture>(uuid);
}

Material* AssetManager::get_material(UUID uuid)
{
    return get_asset<Material>(uuid);
}

void AssetManager::save_assets()
{
    s_assetStorage.save_assets();
}

void AssetManager::save_asset(UUID uuid)
{
    s_assetStorage.save_asset(uuid);
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

void AssetManager::configure_imported_asset(Asset* asset, const ImportContext& importContext)
{
    asset->m_originalFilePath = importContext.originalFilePath;
    asset->m_assetPath = generate_path(importContext.projectDirectory, asset->get_name());
    AssetRegistry::register_asset(asset);
    s_assetStorage.add_asset(asset);
}

std::string AssetManager::generate_path(const std::string& projectDirectory, const std::string& name)
{
    return projectDirectory + "/" + name + ".feasset";
}

}
