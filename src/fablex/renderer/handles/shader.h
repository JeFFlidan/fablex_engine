#pragma once

#include "handle_base.h"
#include "renderer/rhi_types.h"

namespace fe::renderer
{

namespace detail
{

FE_DEFINE_RHI_RESOURCE_TRAITS(
    rhi::Shader, 
    rhi::create_shader, 
    rhi::destroy_shader
);

}

using ShaderHandle = detail::HandleBase<rhi::Shader, ShaderCreateInfo>;
using ShaderRef = detail::RefBase<rhi::Shader>;

}
