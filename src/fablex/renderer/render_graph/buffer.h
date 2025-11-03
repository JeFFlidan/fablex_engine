#pragma once

#include "core/types.h"
#include "core/name.h"
#include "handles/buffer.h"
#include "rhi/resources/buffer.h"

namespace fe::renderer::rg
{

class Buffer
{
public:
    Buffer() = default;
    Buffer(const BufferCreateInfo& createInfo, Name bufferName);
    ~Buffer();

    BufferRef handle() const { return m_handle; }
    uint32 descriptor() const { return m_handle->descriptorIndex; }

private:
    Name m_name;
    BufferHandle m_handle;
};

}