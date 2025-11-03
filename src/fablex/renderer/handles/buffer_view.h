#pragma once

#include "handle_base.h"
#include "renderer/rhi_types.h"

namespace fe::renderer
{

namespace detail
{

FE_DEFINE_RHI_RESOURCE_TRAITS(
    rhi::BufferView, 
    rhi::create_buffer_view, 
    rhi::destroy_buffer_view
);

}

using BufferViewHandle = detail::HandleBase<rhi::BufferView, BufferViewCreateInfo>;
using BufferViewRef = detail::RefBase<rhi::BufferView>;

}