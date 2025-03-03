#pragma once

#include "core/types.h"
#include "rhi/resources.h"

namespace fe::renderer
{

class Buffer
{
public:
    Buffer() = default;
    Buffer(rhi::BufferHandle handle);
    ~Buffer();

    const rhi::BufferHandle get_handle() const { return m_handle; }
    uint32 get_descriptor() const { return m_handle->descriptorIndex; }

private:
    rhi::BufferHandle m_handle = nullptr;
};

}