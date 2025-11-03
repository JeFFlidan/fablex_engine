#pragma once

#include "handle_base.h"
#include "synchronization.h"
#include "renderer/rhi_types.h"

namespace fe::renderer
{

namespace detail
{

FE_DEFINE_RHI_RESOURCE_TRAITS(
    rhi::SwapChain, 
    rhi::create_swap_chain, 
    rhi::destroy_swap_chain
);

template<typename Base>
class SwapChainInterface : public Base
{
public:
    using Base::Base;

    uint32 acquire_next_image(SemaphoreRef signalSemaphore, FenceRef fence)
    {
        uint32 index = 0;
        rhi::acquire_next_image(this->m_resource, signalSemaphore, fence, &index);
        return index;
    }
};

using SwapChainHandleBase = HandleBase<rhi::SwapChain, SwapChainCreateInfo>;
using SwapChainRefBase = RefBase<rhi::SwapChain>;
using SwapChainHandleInterface = SwapChainInterface<SwapChainHandleBase>;
using SwapChainRefInterface = SwapChainInterface<SwapChainRefBase>;

}

using SwapChainHandle = detail::SwapChainHandleInterface;
using SwapChainRef = detail::SwapChainRefInterface;

}