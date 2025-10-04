#pragma once

#include "core/types.h"
#include "core/name.h"
#include "rhi/resources/buffer.h"

namespace fe::renderer::rg
{

class Buffer
{
public:
    Buffer() = default;
    Buffer(rhi::Buffer* handle, Name bufferName);
    ~Buffer();

    rhi::Buffer* handle() const { return m_handle; }
    uint32 descriptor() const { return m_handle->descriptorIndex; }

private:
    Name m_name;
    rhi::Buffer* m_handle = nullptr;
};

}