#pragma once

#include "handle_base.h"
#include "renderer/rhi_types.h"

namespace fe::renderer
{

namespace detail
{

FE_DEFINE_RHI_RESOURCE_TRAITS(
    rhi::Sampler, 
    rhi::create_sampler, 
    rhi::destroy_sampler
);

}

using SamplerHandle = detail::HandleBase<rhi::Sampler, SamplerCreateInfo>;
using SamplerRef = detail::RefBase<rhi::Sampler>;

}
