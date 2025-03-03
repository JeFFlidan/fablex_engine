#include "buffer.h"
#include "rhi/rhi.h"

namespace fe::renderer
{

Buffer::Buffer(rhi::BufferHandle handle) : m_handle(handle) { }

Buffer::~Buffer()
{
    if (m_handle) rhi::destroy_buffer(m_handle);
}

}