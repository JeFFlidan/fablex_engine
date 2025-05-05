#pragma once

#include "asset_manager/fwd.h"
#include "rhi/resources.h"

namespace fe::renderer
{

class GPUTexture
{
public:
    asset::Texture* get_texture() const { return m_texture; }
    rhi::Texture* get_gpu_texture() const { return m_gpuTexture; }
    rhi::TextureView* get_gpu_texture_view() const { return m_gpuTextureView; }

private:
    asset::Texture* m_texture = nullptr;
    rhi::Texture* m_gpuTexture = nullptr;
    rhi::TextureView* m_gpuTextureView = nullptr;
};

}