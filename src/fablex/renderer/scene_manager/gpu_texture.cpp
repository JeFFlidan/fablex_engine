#include "gpu_texture.h"
#include "command_recorder.h"
#include "rhi/rhi.h"
#include "asset_manager/texture/texture.h"

namespace fe::renderer
{

GPUTexture::GPUTexture(asset::Texture* textureAsset)
    : m_textureAsset(textureAsset)
{

}

GPUTexture::~GPUTexture()
{
    rhi::destroy_texture_view(m_textureView);
    rhi::destroy_texture(m_texture);
}

void GPUTexture::create(uint32 mipLevels)
{
    rhi::TextureInfo textureInfo;
    textureInfo.width = m_textureAsset->width();
    textureInfo.height = m_textureAsset->height();
    textureInfo.depth = m_textureAsset->depth();
    textureInfo.dimension = get_dimension();   // TEMP
    textureInfo.format = m_textureAsset->format();
    textureInfo.layersCount = 1;
    textureInfo.memoryUsage = rhi::MemoryUsage::GPU;
    textureInfo.mipLevels = mipLevels;
    textureInfo.textureUsage = rhi::ResourceUsage::SAMPLED_TEXTURE | rhi::ResourceUsage::TRANSFER_DST;
    textureInfo.samplesCount = rhi::SampleCount::BIT_1;
    rhi::create_texture(&m_texture, &textureInfo);

    rhi::TextureViewInfo textureViewInfo;
    textureViewInfo.type = rhi::ViewType::SRV;
    textureViewInfo.aspect = rhi::TextureAspect::COLOR;
    textureViewInfo.baseLayer = 0;
    textureViewInfo.layerCount = 1;
    textureViewInfo.baseMipLevel = 0;
    textureViewInfo.mipLevels = mipLevels;
    rhi::create_texture_view(&m_textureView, &textureViewInfo, m_texture);

    rhi::set_name(m_texture, m_textureAsset->get_name());
    rhi::set_name(m_textureView, m_textureAsset->get_name() + "View");
}

void GPUTexture::build(const CommandRecorder& cmdRecorder, const rhi::TextureInitInfo& initInfo)
{
    cmdRecorder.record([&](rhi::CommandBuffer* cmd)
    {
        rhi::init_texture(cmd, m_texture, &initInfo);
    });
}

rhi::TextureDimension GPUTexture::get_dimension() const
{
    return m_textureAsset->depth() == 0 ? rhi::TextureDimension::TEXTURE1D
        : m_textureAsset->depth() == 1 ? rhi::TextureDimension::TEXTURE2D 
        : rhi::TextureDimension::TEXTURE3D;
}

}