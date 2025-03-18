#include "buffer.h"
#include "rhi/rhi.h"

namespace fe::renderer
{

Buffer::Buffer(rhi::BufferHandle handle, Name bufferName) 
    : m_name(bufferName), m_handle(handle)
{
    rhi::set_name(m_handle, m_name.to_string());
}

Buffer::~Buffer()
{
    if (m_handle) rhi::destroy_buffer(m_handle);
}

}