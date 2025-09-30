#pragma once

#include "core/types.h"
#include "core/name.h"
#include "rhi/resources.h"

namespace fe::renderer::rg
{

class Buffer
{
public:
    Buffer() = default;
    Buffer(rhi::BufferHandle handle, Name bufferName);
    ~Buffer();

    rhi::BufferHandle handle() const { return m_handle; }
    uint32 descriptor() const { return m_handle->descriptorIndex; }

private:
    Name m_name;
    rhi::BufferHandle m_handle = nullptr;
};

}