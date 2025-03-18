#include "resource.h"
#include "core/macro.h"

namespace fe::renderer
{

Resource::Resource(ResourceName name, uint64 viewCount) 
    : m_name(name), m_schedulingInfo(name, viewCount), m_viewCount(viewCount)
{

}

void Resource::set_texture(rhi::TextureHandle textureHandle)
{
    m_texture.reset(new Texture(textureHandle, m_name));
}

void Resource::set_buffer(rhi::BufferHandle bufferHanlde)
{
    m_buffer.reset(new Buffer(bufferHanlde, m_name));
}

void Resource::set_from_resource(Resource& other)
{
    if (other.is_buffer())
        m_buffer = std::move(other.m_buffer);
    if (other.is_texture())
        m_texture = std::move(other.m_texture);
}

bool Resource::IntersectionEntry::operator<(const IntersectionEntry& other) const
{
    return name.to_id() < other.name.to_id();
}

}