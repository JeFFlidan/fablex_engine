#include "asset_manager_test.h"
#include "asset_manager.h"
#include "core/file_system/file_system.h"

namespace fe::asset
{

void AssetManagerTest::run()
{
    std::string projectDirectory = "projects/asset_manager_test";
    FileSystem::create_project_directory(projectDirectory);

    std::vector<const AssetData*> modelAssetDatas;
    // AssetRegistry::get_assets_by_type(Type::MODEL, modelAssetDatas);

    FE_LOG(LogAssetManager, INFO, "Model AssetData count {}", modelAssetDatas.size());

    for (const AssetData* assetData : modelAssetDatas)
    {
        FE_LOG(LogAssetManager, INFO, "Model Name: {}", assetData->name);
        FE_LOG(LogAssetManager, INFO, "Model Path: {}", assetData->path);
        FE_LOG(LogAssetManager, INFO, "Model UUID: {}", assetData->uuid);
    }

    std::vector<const AssetData*> textureAssetDatas;
    // AssetRegistry::get_assets_by_type(Type::TEXTURE, textureAssetDatas);

    FE_LOG(LogAssetManager, INFO, "Texture AssetData count {}", textureAssetDatas.size());

    for (const AssetData* assetData : textureAssetDatas)
    {
        FE_LOG(LogAssetManager, INFO, "Texture Name: {}", assetData->name);
        FE_LOG(LogAssetManager, INFO, "Texture Path: {}", assetData->path);
        FE_LOG(LogAssetManager, INFO, "Texture UUID: {}", assetData->uuid);
    }

    if (modelAssetDatas.empty())
    {
        ModelImportContext importContext;
        importContext.originalFilePath = FileSystem::get_absolute_path("content/cubes.glb");
        importContext.projectDirectory = projectDirectory;
        importContext.mergeMeshes = false;
        ModelImportResult importResult;

        bool result = AssetManager::import_model(importContext, importResult);
        FE_CHECK(result);

        FE_LOG(LogAssetManager, INFO, "Model count: {}", importResult.models.size());
        
        for (const Model* model : importResult.models)
        {
            FE_LOG(LogAssetManager, INFO, "Model {} vertex count: {}", model->get_name(), model->vertex_positions().size());
            AssetManager::save_asset(model->get_uuid());
        }
    }
    else
    {
        FE_LOG(LogAssetManager, INFO, "Start getting models.");
        for (const AssetData* assetData : modelAssetDatas)
        {
            Model* model = AssetManager::get_model(assetData->uuid);

            FE_LOG(LogAssetManager, INFO, "Model Name: {}", model->get_name());
            FE_LOG(LogAssetManager, INFO, "Model Original File Path: {}", model->get_original_file_path());
        }
    }

    if (textureAssetDatas.empty())
    {
        FE_LOG(LogAssetManager, INFO, "Start texture loading");
        TextureImportContext textureImportContext;
        textureImportContext.originalFilePath = FileSystem::get_absolute_path("content/BrickWall_BaseColor.jpg");
        textureImportContext.projectDirectory = projectDirectory;
        TextureImportResult textureImportResult;
        
        bool result = AssetManager::import_texture(textureImportContext, textureImportResult);
    
        FE_LOG(LogAssetManager, INFO, "Finished texture import");
        FE_CHECK(result);
    
        Texture* texture = textureImportResult.texture;
        FE_LOG(LogAssetManager, INFO, "Texture size: {} {}", texture->width(), texture->height());
        AssetManager::save_asset(texture->get_uuid());
    }
    else
    {
        FE_LOG(LogAssetManager, INFO, "Start getting textures.");
        for (const AssetData* assetData : textureAssetDatas)
        {
            Texture* texture = AssetManager::get_texture(assetData->uuid);

            FE_LOG(LogAssetManager, INFO, "Texture Name: {}", texture->get_name());
            FE_LOG(LogAssetManager, INFO, "Texture Original File Path: {}", texture->get_original_file_path());
        }
    }
}

}