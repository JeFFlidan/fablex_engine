#include "gpu_texture.h"
#include "command_recorder.h"
#include "scene_manager.h"
#include "rhi/rhi.h"
#include "asset_manager/texture/texture.h"

namespace fe::renderer
{

constexpr uint32 UPLOADED_TEXTURE_INDEX = 333;

GPUTexture::GPUTexture(asset::Texture* textureAsset) : GPUResource(textureAsset)
{

}

GPUTexture::~GPUTexture()
{
    rhi::destroy_texture_view(m_textureView);
    rhi::destroy_texture(m_texture);
}

void GPUTexture::reset()
{
    m_refCount = 0;
}

void GPUTexture::build(SceneManager* sceneManager)
{
    rhi::TextureInfo textureInfo;
    textureInfo.width = m_asset->width();
    textureInfo.height = m_asset->height();
    textureInfo.depth = m_asset->depth();
    textureInfo.dimension = get_dimension();   // TEMP
    textureInfo.format = m_asset->format();
    textureInfo.layersCount = 1;
    textureInfo.memoryUsage = rhi::MemoryUsage::GPU;
    textureInfo.mipLevels = m_asset->mipmaps().size();
    textureInfo.textureUsage = rhi::ResourceUsage::SAMPLED_TEXTURE | rhi::ResourceUsage::TRANSFER_DST;
    textureInfo.samplesCount = rhi::SampleCount::BIT_1;
    rhi::create_texture(&m_texture, &textureInfo);

    rhi::TextureViewInfo textureViewInfo;
    textureViewInfo.type = rhi::ViewType::SRV;
    textureViewInfo.aspect = rhi::TextureAspect::COLOR;
    textureViewInfo.baseLayer = 0;
    textureViewInfo.layerCount = 1;
    textureViewInfo.baseMipLevel = 0;
    textureViewInfo.mipLevels = m_asset->mipmaps().size();
    rhi::create_texture_view(&m_textureView, &textureViewInfo, m_texture);

    rhi::set_name(m_texture, m_asset->get_name());
    rhi::set_name(m_textureView, m_asset->get_name() + "View");
}

bool GPUTexture::upload_to_gpu(const SceneManager* sceneManager)
{
    // Transient resources are built in (editor icons, etc.), so we don't need to remove them if the ref count is 0
    if (m_refCount == 0 && !asset()->has_flag(asset::AssetFlag::TRANSIENT))
        return false;

    if (m_indexInBuffer != UPLOADED_TEXTURE_INDEX)
    {
        sceneManager->record_graphics_cmd([this](rhi::CommandBuffer* cmd)
        {
            rhi::TextureInitInfo initInfo;
            initInfo.buffer = m_asset->upload_buffer();
            initInfo.mipMaps = m_asset->mipmaps();
            rhi::init_texture(cmd, m_texture, &initInfo);

            m_indexInBuffer = UPLOADED_TEXTURE_INDEX;
        });
    }

    return true;
}

rhi::TextureDimension GPUTexture::get_dimension() const
{
    return m_asset->depth() == 0 ? rhi::TextureDimension::TEXTURE1D
        : m_asset->depth() == 1 ? rhi::TextureDimension::TEXTURE2D 
        : rhi::TextureDimension::TEXTURE3D;
}

}