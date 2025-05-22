#pragma once

#include "texture.h"
#include "dds-ktx.h"

namespace fe::asset
{

class TextureBridge
{
public:
    static bool import(const TextureImportContext& inImportContext, TextureImportResult& outImportResult);
    static bool import(const TextureImportFromMemoryContext& inImportContext, TextureImportResult& outImportResult);

private:
    inline static bool s_isBasisuInitialized = false;

    static bool load_texture(const TextureImportFromMemoryContext& inImportContext, TextureImportResult& outImportResult);

    static rhi::Format dds_format_to_internal(ddsktx_format ddsKtxFormat);
    static rhi::Buffer* create_upload_buffer(uint32 bufferSize);

    template<typename TextureFormat>
    static uint64 get_texture_size(int width, int height, int channelCount)
    {
        return width * height * (channelCount == 3 ? channelCount + 1 : channelCount) * sizeof(TextureFormat);
    }
};

}