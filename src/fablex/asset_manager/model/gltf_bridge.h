#pragma once

#include "model.h"

namespace fe::asset
{

class GLTFBridge
{
public:
    static bool import(const ModelImportContext& inImportContext, ModelImportResult& outImportResult);
};

}