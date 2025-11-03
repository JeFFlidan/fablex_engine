#include "buffer.h"
#include "rhi/rhi.h"
#include "rhi/resources/buffer.h"

namespace fe::renderer::rg
{

Buffer::Buffer(const BufferCreateInfo& createInfo, Name bufferName) 
    : m_name(bufferName)
{
    m_handle.init(createInfo);
    m_handle.set_name(m_name.to_string());
}

Buffer::~Buffer()
{
    
}

}