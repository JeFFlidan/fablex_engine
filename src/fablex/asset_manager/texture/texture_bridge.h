#pragma once

#include "texture.h"
#include "dds-ktx.h"

namespace fe::asset
{

class TextureBridge
{
public:
    static bool import(const TextureImportContext& inImportContext, TextureImportResult& outImportResult);

private:
    inline static bool s_isBasisuInitialized = false;

    static rhi::Format dds_format_to_internal(ddsktx_format ddsKtxFormat);
    static rhi::Buffer* create_upload_buffer(uint32 bufferSize);
};

}