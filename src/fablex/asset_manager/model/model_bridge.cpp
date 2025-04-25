#define FE_MODEL_PROXY
#include "model_bridge.h"
#include "gltf_bridge.h"
#include "core/file_system/file_system.h"

namespace fe::asset
{

bool ModelBridge::import(const ModelImportContext& inImportContext, ModelImportResult& outImportResult)
{
    const std::string extension = FileSystem::get_file_extension(inImportContext.originalFilePath);
    
    if (extension == "gltf" || extension == "glb")
    {
        if (!GLTFBridge::import(inImportContext, outImportResult))
        {
            FE_LOG(LogAssetManager, ERROR, "Failed to import 3D model: {}.", inImportContext.originalFilePath);
            return false;
        }
    }

    for (Model* model : outImportResult.models)
    {
        ModelProxy modelProxy(model);

        modelProxy.aabb.minPoint = Float3(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);
        modelProxy.aabb.maxPoint = Float3(FLOAT_MIN, FLOAT_MIN, FLOAT_MIN);

        for (const Float3& position : model->vertex_positions())
        {
            modelProxy.aabb.minPoint = min(modelProxy.aabb.minPoint, position);
            modelProxy.aabb.maxPoint = max(modelProxy.aabb.maxPoint, position);
        }
    }

    return true;
}

}