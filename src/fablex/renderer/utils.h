#pragma once

#include "core/types.h"
#include "asset_manager/fwd.h"

namespace fe::renderer
{

class RenderContext;

class Utils
{
public:
    static void init(RenderContext* renderContext);

    static int32 get_descriptor(asset::Texture* textureAsset);
    static int32 get_sampler_linear_repeat();
    static int32 get_sampler_linear_clamp();
    static int32 get_sampler_linear_mirror();
    static int32 get_sampler_nearest_repeat();
    static int32 get_sampler_nearest_clamp();
    static int32 get_sampler_nearest_mirror();
    static int32 get_sampler_minimum_nearest_clamp();

private:
    inline static RenderContext* s_renderContext = nullptr;

    static int32 get_sampler_descriptor(const char* samplerName);
};

}