#include "model_bridge.h"
#include "gltf_bridge.h"
#include "core/file_system/file_system.h"

namespace fe::asset
{

bool ModelBridge::import(const ModelImportContext& inImportContext, ModelImportResult& outImportResult)
{
    const std::string extension = FileSystem::get_file_extension(inImportContext.path);
    
    if (extension == "gltf" || extension == "glb")
    {
        if (!GLTFBridge::import(inImportContext, outImportResult))
        {
            FE_LOG(LogAssetManager, ERROR, "Failed to import 3D model: {}.", inImportContext.path);
            return false;
        }
    }

    return true;
}

}