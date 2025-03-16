#pragma once

#include "common.h"
#include "buffer.h"
#include "texture.h"
#include "resource_scheduling_info.h"

namespace fe::renderer
{

class Resource
{
public:
    struct IntersectionEntry
    {
        ResourceName name;

        bool operator==(const IntersectionEntry& other) const;
    };

    Resource(ResourceName name, uint64 viewCount);

    bool is_valid() const { return m_texture.get_handle() || m_buffer.get_handle(); }
    bool is_buffer() const { return m_buffer.get_handle(); }
    bool is_texture() const { return m_texture.get_handle(); }
    
    ResourceName get_name() const { return m_name; }
    Buffer& get_buffer() { return m_buffer; }
    const Buffer& get_buffer() const { return m_buffer; }
    Texture& get_texture() { return m_texture;}
    const Texture& get_texture() const { return m_texture; }
    ResourceSchedulingInfo& get_scheduling_info() { return m_schedulingInfo; }
    const ResourceSchedulingInfo& get_scheduling_info() const { return m_schedulingInfo; }
    uint64 get_view_count() const { return m_viewCount; }

    void set_buffer(Buffer&& buffer);
    void set_texture(Texture&& textureHandle);

private:
    ResourceName m_name;
    Buffer m_buffer;
    Texture m_texture;
    ResourceSchedulingInfo m_schedulingInfo;
    uint64 m_viewCount;
};

}