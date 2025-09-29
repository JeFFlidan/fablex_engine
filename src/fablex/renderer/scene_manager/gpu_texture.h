#pragma once

#include "gpu_resource.h"
#include "rhi/resources.h"
#include "asset_manager/texture/texture.h"

namespace fe::renderer
{

class SceneManager;

class GPUTexture : public GPUResource<GPUTexture, asset::Texture>
{
public:
    GPUTexture(asset::Texture* textureAsset);
    virtual ~GPUTexture() override;

    void reset();
    
    virtual void build(SceneManager* sceneManager) override;
    virtual bool upload_to_gpu(const SceneManager* sceneManager) override;

    rhi::Texture* texture() const { return m_texture; }
    rhi::TextureView* texture_view() const { return m_textureView; }
    uint32 descriptor() const { return m_textureView->descriptorIndex; }

private:
    rhi::Texture* m_texture = nullptr;
    rhi::TextureView* m_textureView = nullptr;

    rhi::TextureDimension get_dimension() const;
};

}