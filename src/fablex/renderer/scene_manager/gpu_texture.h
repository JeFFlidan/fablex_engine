#pragma once

#include "asset_manager/fwd.h"
#include "rhi/resources.h"

namespace fe::renderer
{

class CommandRecorder;

class GPUTexture
{
public:
    GPUTexture(asset::Texture* textureAsset);
    ~GPUTexture();
    
    void create(uint32 mipLevels);
    void build(const CommandRecorder& cmdRecorder, const rhi::TextureInitInfo& initInfo);

    asset::Texture* texture_asset() const { return m_textureAsset; }
    rhi::Texture* texture() const { return m_texture; }
    rhi::TextureView* texture_view() const { return m_textureView; }
    uint32 descriptor() const { return m_textureView->descriptorIndex; }

private:
    asset::Texture* m_textureAsset = nullptr;
    rhi::Texture* m_texture = nullptr;
    rhi::TextureView* m_textureView = nullptr;

    rhi::TextureDimension get_dimension() const;
};

}