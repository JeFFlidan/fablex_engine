#pragma once

#include "gpu_resource.h"
#include "handles/texture.h"
#include "handles/texture_view.h"
#include "asset_manager/texture/texture.h"

namespace fe::renderer
{

class SceneManager;

class GPUTexture : public GPUResource<GPUTexture, asset::Texture>
{
public:
    GPUTexture(asset::Texture* textureAsset);
    virtual ~GPUTexture() override;

    virtual void reset() override;
    
    virtual void build(SceneManager* sceneManager) override;
    virtual bool upload_to_gpu(const SceneManager* sceneManager) override;

    TextureRef texture() const { return m_texture; }
    TextureViewRef texture_view() const { return m_textureView; }
    uint32 descriptor() const { return m_textureView->descriptorIndex; }

private:
    TextureHandle m_texture;
    TextureViewHandle m_textureView;

    TextureDimension get_dimension() const;
};

}