#include "asset_manager_test.h"
#include "model/model.h"
#include "model/model_bridge.h"
#include "core/file_system/file_system.h"

namespace fe::asset
{

void AssetManagerTest::run()
{
    ModelImportContext importContext;
    importContext.path = FileSystem::get_absolute_path("content/cubes.glb");
    importContext.mergeMeshes = false;
    ModelImportResult importResult;
    bool result = ModelBridge::import(importContext, importResult);
    FE_CHECK(result);

    FE_LOG(LogAssetManager, INFO, "Model count: {}", importResult.models.size());
    
    for (const Model* model : importResult.models)
    {
        FE_LOG(LogAssetManager, INFO, "Model {} vertex count: {}", model->get_name(), model->vertex_positions().size());
    }
}

}