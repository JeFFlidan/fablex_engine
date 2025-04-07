#pragma once

#include "model.h"

namespace fe::asset
{

class ModelBridge
{
public:
    static bool import(const ModelImportContext& inImportContext, ModelImportResult& outImportResult);

};

}