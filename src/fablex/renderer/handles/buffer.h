#pragma once

#include "handle_base.h"
#include "globals.h"
#include "rhi/resources/buffer.h"
#include "renderer/rhi_types.h"

namespace fe::renderer
{

namespace detail
{

FE_DEFINE_RHI_RESOURCE_TRAITS(
    rhi::Buffer, 
    rhi::create_buffer, 
    rhi::destroy_buffer
);

template<typename Base, typename BufferValueType = void>
class BufferInterfaceImpl : public Base
{
public:
    using Base::Base;

    void update(uint64 size, const void* data)
    {
        rhi::update_buffer(this->m_resource, size, data);
    }

    // If size = 0, buffer size will be taken
    void bind_uniform_buffer(uint32 slot, uint32 size = 0, uint32 offset = 0)
    {
        rhi::bind_uniform_buffer(
            this->m_resource,
            g_frameIndex,
            slot,
            size ? size : this->m_resource->size,
            offset
        );
    }

    bool has_usage(ResourceUsage usage) const
    {
        return ::has_flag(this->m_resource->bufferUsage, usage);
    }

    bool has_flag(ResourceFlags flag) const
    {
        return ::has_flag(this->m_resource->flags, flag);
    }

    BufferValueType* operator[](uint32 index) const
    {
        if constexpr (!std::is_same_v<BufferValueType, void>)
        {
            BufferValueType* dataArray = static_cast<BufferValueType*>(this->m_resource->mappedData);
            FE_CHECK(this->m_resource->size >= (index + 1) * sizeof(BufferValueType));
            return &dataArray[index];
        }
        
        return nullptr;
    }

    template<typename T = uint8>
    T* mapped_data() const
    {
        return static_cast<T*>(this->m_resource->mappedData);
    }
};

using BufferHandleBase = HandleBase<rhi::Buffer, BufferCreateInfo>;
using BufferRefBase = RefBase<rhi::Buffer>;

using BufferHandleInterface = BufferInterfaceImpl<BufferHandleBase, void>;
using BufferRefInterface = BufferInterfaceImpl<BufferRefBase, void>;

template<typename BufferValueType = void>
using TBufferHandleInterface = BufferInterfaceImpl<BufferHandleBase, BufferValueType>;

template<typename BufferValueType = void>
using TBufferRefInterface = BufferInterfaceImpl<BufferRefBase, BufferValueType>;

}

using BufferHandle = detail::BufferHandleInterface;
using BufferRef = detail::BufferRefInterface;

template<typename BufferValueType = void>
using TBufferHandle = detail::TBufferHandleInterface<BufferValueType>;

template<typename BufferValueType = void>
using TBufferRef = detail::TBufferRefInterface<BufferValueType>;

}