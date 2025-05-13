#include "utils.h"
#include "globals.h"
#include "render_context.h"
#include "scene_manager/scene_manager.h"
#include "asset_manager/texture/texture.h"

namespace fe::renderer
{

void Utils::init(RenderContext* renderContext)
{
    FE_CHECK(renderContext);
    s_renderContext = renderContext;
}

int32 Utils::get_descriptor(asset::Texture* textureAsset)
{
    FE_CHECK(s_renderContext);
    return s_renderContext->scene_manager()->descriptor(textureAsset);
}

int32 Utils::get_sampler_linear_repeat()
{
    return get_sampler_descriptor(SAMPLER_LINEAR_REPEAT);
}

int32 Utils::get_sampler_linear_clamp()
{
    return get_sampler_descriptor(SAMPLER_LINEAR_CLAMP);
}

int32 Utils::get_sampler_linear_mirror()
{
    return get_sampler_descriptor(SAMPLER_LINEAR_MIRROR);
}

int32 Utils::get_sampler_nearest_repeat()
{
    return get_sampler_descriptor(SAMPLER_NEAREST_REPEAT);
}

int32 Utils::get_sampler_nearest_clamp()
{
    return get_sampler_descriptor(SAMPLER_NEAREST_CLAMP);
}

int32 Utils::get_sampler_nearest_mirror()
{
    return get_sampler_descriptor(SAMPLER_NEAREST_MIRROR);
}

int32 Utils::get_sampler_minimum_nearest_clamp()
{
    return get_sampler_descriptor(SAMPLER_MINIMUM_NEAREST_CLAMP);
}

int32 Utils::get_sampler_descriptor(const char* samplerName)
{
    FE_CHECK(s_renderContext);
    return s_renderContext->scene_manager()->sampler_descriptor(samplerName);
}
    
}