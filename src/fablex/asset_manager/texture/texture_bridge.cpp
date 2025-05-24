#define FE_TEXTURE_PROXY
#define DDSKTX_IMPLEMENT
#include "texture_bridge.h"
#include "asset_manager/asset_manager.h"
#include "core/color.h"
#include "core/file_system/file_system.h"
#include "rhi/rhi.h"
#include "rhi/utils.h"
#include "core/event.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "transcoder/basisu_transcoder.h"

namespace fe::asset
{

bool TextureBridge::import(const TextureImportContext& inImportContext, TextureImportResult& outImportResult)
{
    if (!s_isBasisuInitialized)
    {
        basist::basisu_transcoder_init();
        s_isBasisuInitialized = true;
    }

    std::vector<uint8_t> textureRawData;
    FileSystem::read(inImportContext.originalFilePath, textureRawData);

    TextureImportFromMemoryContext importFromMemoryContext;
    importFromMemoryContext.data = textureRawData.data();
    importFromMemoryContext.dataSize = textureRawData.size();
    importFromMemoryContext.name = FileSystem::get_file_name(inImportContext.originalFilePath);
    importFromMemoryContext.originalFilePath = inImportContext.originalFilePath;
    importFromMemoryContext.projectDirectory = inImportContext.projectDirectory;

    return load_texture(importFromMemoryContext, outImportResult);
}

bool TextureBridge::import(const TextureImportFromMemoryContext& inImportContext, TextureImportResult& outImportResult)
{
    return load_texture(inImportContext, outImportResult);
}

bool TextureBridge::load_texture(const TextureImportFromMemoryContext& inImportContext, TextureImportResult& outImportResult)
{
    std::string extension = FileSystem::get_file_extension(inImportContext.originalFilePath);

    TextureCreateInfo createInfo;
    createInfo.name = FileSystem::get_file_name(inImportContext.name);
    outImportResult.texture = AssetManager::create_texture(createInfo);
    TextureProxy textureProxy(outImportResult.texture);

    const uint8* data = static_cast<const uint8*>(inImportContext.data);
    uint64 dataSize = inImportContext.dataSize;

    if (extension == "basis")
    {
        // TODO
    }
    else if (extension == "ktx2")
    {
        // TODO
    }
    else if (extension == "hdr")
    {
        // TODO
    }
    else if (extension == "dds")
    {
        ddsktx_texture_info ddsTextureInfo;
        ddsktx_error error;

        if (!ddsktx_parse(&ddsTextureInfo, data, (int)dataSize, &error))
        {
            FE_LOG(LogAssetManager, ERROR, "Failed to load dds texture {}. Parser error: {}", inImportContext.originalFilePath, error.msg);
            return false;
        }

        FE_CHECK_MSG(ddsTextureInfo.num_layers == 1, "Texture arrays are not supported");

        textureProxy.width = ddsTextureInfo.width;
        textureProxy.height = ddsTextureInfo.height;
        textureProxy.depth = ddsTextureInfo.depth;
        textureProxy.format = dds_format_to_internal(ddsTextureInfo.format);
        textureProxy.bcFormat = rhi::is_format_block_compressed(textureProxy.format) 
            ? textureProxy.format : rhi::Format::UNDEFINED;

        textureProxy.mipmaps.reserve(ddsTextureInfo.num_mips);
        textureProxy.uploadBuffer = create_upload_buffer(ddsTextureInfo.size_bytes, inImportContext.name);

        uint32 bufferOffset = 0;
        uint8* bufferMappedData = (uint8*)textureProxy.uploadBuffer->mappedData;

        for (uint32 mip = 0; mip != ddsTextureInfo.num_mips; ++mip)
        {
            uint32 mipLevelSize = 0;

            for (uint32 depthLayer = 0; depthLayer != ddsTextureInfo.depth; ++depthLayer)
            {
                ddsktx_sub_data subData;
                ddsktx_get_sub(&ddsTextureInfo, &subData, data, (int)dataSize, 0, depthLayer, mip);

                mipLevelSize += subData.size_bytes;
                const uint32 blockSize = rhi::get_block_format_size(textureProxy.format);
                const uint32 numBlocksX = std::max(1u, subData.width / blockSize);
                const uint32 numBlockY = std::max(1u, subData.height / blockSize);
                const uint32 dstRowPitch = numBlocksX * rhi::get_format_stride(textureProxy.format);
                const uint32 dstSlicePitch = dstRowPitch * numBlockY;
                const uint32 srcRowPitch = subData.row_pitch_bytes;
                // const uint32 srcSlicePitch = subData.size_bytes;

                uint8* srcSlice = (uint8*)subData.buff;
                uint8* dstSlice = bufferMappedData + bufferOffset + dstSlicePitch * depthLayer;

                for (uint32 y = 0; y != numBlockY; ++y)
                {
                    memcpy(
                        dstSlice + dstRowPitch * y,
                        srcSlice + srcRowPitch * y,
                        dstRowPitch
                    );
                }
            }
            rhi::MipMap& mipMap = textureProxy.mipmaps.emplace_back();
            mipMap.layer = 0;   // TEMP
            mipMap.offset = bufferOffset;

            bufferOffset += mipLevelSize;
        }
    }
    else
    {
        int width = 0, height = 0, channels = 0;

        // stbi has no mipmaps, but I need one default mipmap to describe what value to copy from the upload buffer
        textureProxy.mipmaps.emplace_back();

        if (stbi_is_16_bit_from_memory(data, dataSize))
        {
            void* textureData = stbi_load_16_from_memory(data, dataSize, &width, &height, &channels, 0);
            const uint64 textureSize = get_texture_size<uint16>(width, height, channels);
            
            textureProxy.width = width;
            textureProxy.height = height;
            textureProxy.depth = 1;
            textureProxy.is16Bit = true;

            textureProxy.uploadBuffer = create_upload_buffer(textureSize, inImportContext.name);
            void* uploadBufferData = textureProxy.uploadBuffer->mappedData;

            switch (channels)
            {
                case 1:
                {
                    textureProxy.format = rhi::Format::R16_UNORM;
                    textureProxy.bcFormat = rhi::Format::BC4_UNORM;
                    textureProxy.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::ONE };
                    memcpy(uploadBufferData, textureData, textureSize);
                    break;
                }
                case 2:
                {
                    textureProxy.format = rhi::Format::R16G16_UNORM;
                    textureProxy.bcFormat = rhi::Format::BC5_UNORM;
                    textureProxy.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::ONE, rhi::ComponentSwizzle::ONE };
                    memcpy(uploadBufferData, textureData, textureSize);
                    break;
                }
                case 3:
                {
                    textureProxy.format = rhi::Format::R16G16B16A16_UNORM;
                    textureProxy.bcFormat = rhi::Format::BC1_RGBA_UNORM;
                    textureProxy.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::B, rhi::ComponentSwizzle::ONE };

                    struct Color3Channels
                    {
                        uint16_t r, g, b;
                    };

                    Color3Channels* textureData3Channels = static_cast<Color3Channels*>(textureData);
                    Color16Bit* typedUploadBufferData = static_cast<Color16Bit*>(uploadBufferData);

                    for (size_t i = 0; i < textureSize / sizeof(Color16Bit); ++i)
                        typedUploadBufferData[i] = { textureData3Channels[i].r, textureData3Channels[i].g, textureData3Channels[i].b };

                    break;
                }
                case 4:
                {
                    textureProxy.format = rhi::Format::R16G16B16A16_UNORM;
                    textureProxy.bcFormat = rhi::Format::BC3_UNORM;
                    textureProxy.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::B, rhi::ComponentSwizzle::A };
                    memcpy(uploadBufferData, textureData, textureSize);
                    break;
                }
            }
            
            stbi_image_free(textureData);
        }
        else
        {
            void* textureData = stbi_load_from_memory(data, dataSize, &width, &height, &channels, 0);
            const uint64 textureSize = get_texture_size<uint8>(width, height, channels);

            textureProxy.width = width;
            textureProxy.height = height;
            textureProxy.depth = 1;
            textureProxy.is16Bit = false;

            textureProxy.uploadBuffer = create_upload_buffer(textureSize, inImportContext.name);
            void* uploadBufferData = textureProxy.uploadBuffer->mappedData;

            switch (channels)
            {
                case 1:
                {
                    textureProxy.format = rhi::Format::R8_UNORM;
                    textureProxy.bcFormat = rhi::Format::BC4_UNORM;
                    textureProxy.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::ONE };
                    memcpy(uploadBufferData, textureData, textureSize);
                    break;
                }
                case 2:
                {
                    textureProxy.format = rhi::Format::R8G8_UNORM;
                    textureProxy.bcFormat = rhi::Format::BC5_UNORM;
                    textureProxy.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::ONE, rhi::ComponentSwizzle::ONE };
                    memcpy(uploadBufferData, textureData, textureSize);
                    break;
                }
                case 3:
                {
                    textureProxy.format = rhi::Format::R8G8B8A8_UNORM;
                    textureProxy.bcFormat = rhi::Format::BC1_RGBA_UNORM;
                    textureProxy.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::B, rhi::ComponentSwizzle::ONE };

                    struct Color3Channels
                    {
                        uint8_t r, g, b;
                    };

                    Color3Channels* textureData3Channels = static_cast<Color3Channels*>(textureData);
                    Color8Bit* typedUploadBufferData = static_cast<Color8Bit*>(uploadBufferData);

                    for (size_t i = 0; i < textureSize / sizeof(Color8Bit); ++i)
                        typedUploadBufferData[i] = { textureData3Channels[i].r, textureData3Channels[i].g, textureData3Channels[i].b };

                    break;
                }
                case 4:
                {
                    textureProxy.format = rhi::Format::R8G8B8A8_UNORM;
                    textureProxy.bcFormat = rhi::Format::BC3_UNORM;
                    textureProxy.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::B, rhi::ComponentSwizzle::A };
                    memcpy(uploadBufferData, textureData, textureSize);
                    break;
                }
            }

            stbi_image_free(textureData);
        }
    }

    return true;
}

rhi::Format TextureBridge::dds_format_to_internal(ddsktx_format ddsKtxFormat)
{
    switch (ddsKtxFormat)
    {
    case DDSKTX_FORMAT_R8:      
        return rhi::Format::R8_UNORM;
    case DDSKTX_FORMAT_RGBA8:   
        return rhi::Format::R8G8B8A8_UNORM;
    case DDSKTX_FORMAT_RGBA8S:  
        return rhi::Format::R8G8B8A8_SINT;
    case DDSKTX_FORMAT_RG16:    
        return rhi::Format::R16G16_UINT;
    case DDSKTX_FORMAT_RGB8:    
        return rhi::Format::R8G8B8A8_UINT;
    case DDSKTX_FORMAT_R16:     
        return rhi::Format::R16_UINT;
    case DDSKTX_FORMAT_R32F:    
        return rhi::Format::R32_SFLOAT;
    case DDSKTX_FORMAT_R16F:    
        return rhi::Format::R16_SFLOAT;
    case DDSKTX_FORMAT_RG16F:   
        return rhi::Format::R16G16_SFLOAT;
    case DDSKTX_FORMAT_RG16S:   
        return rhi::Format::R16G16_SINT;
    case DDSKTX_FORMAT_RGBA16F: 
        return rhi::Format::R16G16B16A16_SFLOAT;
    case DDSKTX_FORMAT_RGBA16:  
        return rhi::Format::R16G16B16A16_UNORM;
    case DDSKTX_FORMAT_RG8:     
        return rhi::Format::R8G8_UNORM;
    case DDSKTX_FORMAT_RG8S:    
        return rhi::Format::R8G8_SINT;
    case DDSKTX_FORMAT_BGRA8:   
        return rhi::Format::B8G8R8A8_UNORM;
    case DDSKTX_FORMAT_BC1:     
        return rhi::Format::BC1_RGBA_UNORM;
    case DDSKTX_FORMAT_BC2:     
        return rhi::Format::BC2_UNORM;
    case DDSKTX_FORMAT_BC3:     
        return rhi::Format::BC3_UNORM;
    case DDSKTX_FORMAT_BC4:     
        return rhi::Format::BC4_UNORM;
    case DDSKTX_FORMAT_BC5:     
        return rhi::Format::BC5_UNORM;
    case DDSKTX_FORMAT_BC7:     
        return rhi::Format::BC7_UNORM;
    default:
        FE_CHECK(0);
        return rhi::Format::UNDEFINED;
    }
}

rhi::Buffer* TextureBridge::create_upload_buffer(uint32 bufferSize, const std::string& textureName)
{
    rhi::BufferInfo bufferInfo;
    bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC;
    bufferInfo.memoryUsage = rhi::MemoryUsage::CPU;
    bufferInfo.size = bufferSize;
    rhi::Buffer* buffer;
    rhi::create_buffer(&buffer, &bufferInfo);

    rhi::set_name(buffer, textureName + "UploadBuffer");

    return buffer;
}

}