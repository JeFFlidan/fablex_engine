#include "gpu_texture.h"
#include "command_recorder.h"
#include "scene_manager.h"
#include "asset_manager/texture/texture.h"

namespace fe::renderer
{

constexpr uint32 UPLOADED_TEXTURE_INDEX = 333;

GPUTexture::GPUTexture(asset::Texture* textureAsset) : GPUResource(textureAsset)
{

}

GPUTexture::~GPUTexture()
{

}

void GPUTexture::reset()
{
    m_refCount = 0;
}

void GPUTexture::build(SceneManager* sceneManager)
{
    m_texture.init(
        TextureCreateInfo
        {
            .width = uint32(m_asset->width()),
            .height = uint32(m_asset->height()),
            .depth = uint32(m_asset->depth()),
            .mipLevels = uint32(m_asset->mipmaps().size()),
            .layersCount = 1,
            .format = m_asset->format(),
            .textureUsage = ResourceUsage::SAMPLED_TEXTURE | ResourceUsage::TRANSFER_DST,
            .memoryUsage = MemoryUsage::GPU,
            .samplesCount = SampleCount::BIT_1,
            .dimension = get_dimension(),
        }
    );

    m_textureView.init(
        TextureViewCreateInfo
        {
            .texture = m_texture,
            .baseMipLevel = 0,
            .baseLayer = 0,
            .mipLevels = uint32(m_asset->mipmaps().size()),
            .layerCount = 1,
            .aspect = TextureAspect::COLOR,
            .type = ViewType::SRV,
        }
    );

    m_texture.set_name(m_asset->get_name());
    m_textureView.set_name(m_asset->get_name() + "View");
}

bool GPUTexture::upload_to_gpu(const SceneManager* sceneManager)
{
    // Transient resources are built in (editor icons, etc.), so we don't need to remove them if the ref count is 0
    if (m_refCount == 0 && !asset()->has_flag(asset::AssetFlag::TRANSIENT))
        return false;

    if (m_indexInBuffer != UPLOADED_TEXTURE_INDEX)
    {
        sceneManager->record_graphics_cmd([this](CommandBufferRef cmd)
        {
            TextureInitInfo initInfo;
            initInfo.buffer = m_asset->upload_buffer();
            initInfo.mipMaps = m_asset->mipmaps();
            cmd.init_texture(m_texture, initInfo);

            m_indexInBuffer = UPLOADED_TEXTURE_INDEX;
        });
    }

    return true;
}

TextureDimension GPUTexture::get_dimension() const
{
    return m_asset->depth() == 0 ? TextureDimension::TEXTURE1D
        : m_asset->depth() == 1 ? TextureDimension::TEXTURE2D 
        : TextureDimension::TEXTURE3D;
}

}