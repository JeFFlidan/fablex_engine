#include "resource.h"
#include "core/macro.h"

namespace fe::renderer
{

Resource::Resource(ResourceName name, uint64 viewCount) 
    : m_name(name), m_schedulingInfo(name, viewCount)
{

}

void Resource::set_buffer(Buffer&& buffer)
{
    FE_CHECK(buffer.get_handle());
    m_buffer = std::move(buffer);
}

void Resource::set_texture(Texture&& texture)
{
    FE_CHECK(texture.get_handle());
    m_texture = std::move(texture);
}

bool Resource::IntersectionEntry::operator==(const IntersectionEntry& other) const
{
    return name == other.name;
}

}