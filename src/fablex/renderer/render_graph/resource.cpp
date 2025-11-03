#include "resource.h"

namespace fe::renderer::rg
{

Resource::Resource(ResourceName name, uint64 viewCount) 
    : m_name(name), m_schedulingInfo(name, viewCount), m_viewCount(viewCount)
{

}

void Resource::set_texture(const TextureCreateInfo& createInfo)
{
    m_texture.reset(new Texture(createInfo, m_name));
}

void Resource::set_buffer(const BufferCreateInfo& createInfo)
{
    m_buffer.reset(new Buffer(createInfo, m_name));
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