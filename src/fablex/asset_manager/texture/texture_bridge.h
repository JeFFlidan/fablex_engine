#pragma once

#include "texture.h"

namespace fe::asset
{

class TextureBridge
{
public:
    static bool import(const TextureImportContext& inImportContext, TextureImportResult& outImportResult);

private:
    inline static bool s_isBasisuInitialized = false;
};

}