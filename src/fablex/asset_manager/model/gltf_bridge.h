#pragma once

#include "asset_manager/common.h"

namespace fe::asset
{

class GLTFBridge
{
public:
    static bool import(const ModelImportContext& inImportContext, ModelImportResult& outImportResult);
};

}