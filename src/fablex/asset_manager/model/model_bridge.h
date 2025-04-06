#pragma once

#include "asset_manager/common.h"

namespace fe::asset
{

class ModelBridge
{
public:
    static bool import(const ModelImportContext& inImportContext, ModelImportResult& outImportResult);

};

}