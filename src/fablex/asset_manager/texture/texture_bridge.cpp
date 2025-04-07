#define FE_TEXTURE_PROXY
#include "texture_bridge.h"
#include "asset_manager/asset_manager.h"
#include "core/color.h"
#include "core/file_system/file_system.h"

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
    std::string extension = FileSystem::get_file_extension(inImportContext.originalFilePath);

    TextureCreateInfo createInfo;
    createInfo.name = FileSystem::get_file_name(inImportContext.originalFilePath);
    outImportResult.texture = AssetManager::create_texture(createInfo);
    TextureProxy textureProxy(outImportResult.texture);

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
    else
    {
        int width = 0, height = 0, channels = 0;
        if (stbi_is_16_bit_from_memory(textureRawData.data(), textureRawData.size()))
        {
            void* textureData = stbi_load_16_from_memory(textureRawData.data(), textureRawData.size(), &width, &height, &channels, 0);
            const uint64 textureSize = width * height * channels * sizeof(uint16_t);
            
            textureProxy.width = width;
            textureProxy.height = height;
            textureProxy.is16Bit = true;
            textureProxy.data.resize(textureSize);

            switch (channels)
            {
                case 1:
                {
                    textureProxy.format = rhi::Format::R16_UNORM;
                    textureProxy.bcFormat = rhi::Format::BC4_UNORM;
                    textureProxy.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::ONE };
                    memcpy(textureProxy.data.data(), textureData, textureSize);
                    break;
                }
                case 2:
                {
                    textureProxy.format = rhi::Format::R16G16_UNORM;
                    textureProxy.bcFormat = rhi::Format::BC5_UNORM;
                    textureProxy.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::ONE, rhi::ComponentSwizzle::ONE };
                    memcpy(textureProxy.data.data(), textureData, textureSize);
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

                    for (size_t i = 0; i < textureProxy.data.size() / sizeof(Color16Bit); i += sizeof(Color16Bit))
                    {
                        Color16Bit* color = reinterpret_cast<Color16Bit*>(&textureProxy.data.at(i));
                        *color = { textureData3Channels[i].r, textureData3Channels[i].g, textureData3Channels[i].b };
                    }

                    break;
                }
                case 4:
                {
                    textureProxy.format = rhi::Format::R16G16B16A16_UNORM;
                    textureProxy.bcFormat = rhi::Format::BC3_UNORM;
                    textureProxy.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::B, rhi::ComponentSwizzle::A };
                    memcpy(textureProxy.data.data(), textureData, textureSize);
                    break;
                }
            }
            
            stbi_image_free(textureData);
        }
        else
        {
            void* textureData = stbi_load_from_memory(textureRawData.data(), textureRawData.size(), &width, &height, &channels, 0);
            const uint64 textureSize = width * height * channels;

            textureProxy.width = width;
            textureProxy.height = height;
            textureProxy.is16Bit = false;
            textureProxy.data.resize(textureSize);

            switch (channels)
            {
                case 1:
                {
                    textureProxy.format = rhi::Format::R8_UNORM;
                    textureProxy.bcFormat = rhi::Format::BC4_UNORM;
                    textureProxy.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::ONE };
                    memcpy(textureProxy.data.data(), textureData, textureSize);
                    break;
                }
                case 2:
                {
                    textureProxy.format = rhi::Format::R8G8_UNORM;
                    textureProxy.bcFormat = rhi::Format::BC5_UNORM;
                    textureProxy.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::ONE, rhi::ComponentSwizzle::ONE };
                    memcpy(textureProxy.data.data(), textureData, textureSize);
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

                    for (size_t i = 0; i < textureProxy.data.size() / sizeof(Color8Bit); i += sizeof(Color8Bit))
                    {
                        Color8Bit* color = reinterpret_cast<Color8Bit*>(&textureProxy.data.at(i));
                        *color = { textureData3Channels[i].r, textureData3Channels[i].g, textureData3Channels[i].b };
                    }

                    break;
                }
                case 4:
                {
                    textureProxy.format = rhi::Format::BC3_UNORM;
                    textureProxy.mapping = { rhi::ComponentSwizzle::R, rhi::ComponentSwizzle::G, rhi::ComponentSwizzle::B, rhi::ComponentSwizzle::A };
                    memcpy(textureProxy.data.data(), textureData, textureSize);
                    break;
                }
            }

            stbi_image_free(textureData);
        }
    }

    return true;
}

}