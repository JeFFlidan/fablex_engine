#pragma once

#include "handle_base.h"
#include "renderer/rhi_types.h"

namespace fe::renderer
{

FE_DEFINE_RHI_RESOURCE_RAII(Shader, rhi::create_shader, rhi::destroy_shader);

}
