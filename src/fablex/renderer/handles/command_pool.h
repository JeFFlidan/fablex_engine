#pragma once

#include "handle_base.h"
#include "renderer/rhi_types.h"

namespace fe::renderer
{

namespace detail
{

FE_DEFINE_RHI_RESOURCE_TRAITS(
    rhi::CommandPool, 
    rhi::create_command_pool, 
    rhi::destroy_command_pool
);

template<typename Base>
class CommandPoolInterface : public Base
{
public:
    using Base::Base;

    void reset_pool()
    {
        rhi::reset_command_pool(this->m_resource);
    }
};

using CommandPoolHandleBase = HandleBase<rhi::CommandPool, CommandPoolCreateInfo>;
using CommandPoolRefBase = RefBase<rhi::CommandPool>;

using CommandPoolHandleInterface = CommandPoolInterface<CommandPoolHandleBase>;
using CommandPoolRefInterface = CommandPoolInterface<CommandPoolRefBase>;

}

using CommandPoolHandle = detail::CommandPoolHandleInterface;
using CommandPoolRef = detail::CommandPoolRefInterface;

}
