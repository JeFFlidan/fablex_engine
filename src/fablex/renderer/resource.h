#pragma once

#include "common.h"
#include "buffer.h"
#include "texture.h"
#include "resource_scheduling_info.h"
#include <memory>

namespace fe::renderer
{

class Resource
{
public:
    struct IntersectionEntry
    {
        ResourceName name;

        bool operator<(const IntersectionEntry& other) const;
    };

    Resource(ResourceName name, uint64 viewCount);

    bool is_valid() const { return (m_texture && m_texture->get_handle()) || (m_buffer && m_buffer->get_handle()); }
    bool is_buffer() const { return (m_buffer && m_buffer->get_handle()); }
    bool is_texture() const { return (m_texture && m_texture->get_handle()); }
    
    ResourceName get_name() const { return m_name; }
    Buffer& get_buffer() { return *m_buffer; }
    const Buffer& get_buffer() const { return *m_buffer; }
    Texture& get_texture() { return *m_texture;}
    const Texture& get_texture() const { return *m_texture; }
    ResourceSchedulingInfo& get_scheduling_info() { return m_schedulingInfo; }
    const ResourceSchedulingInfo& get_scheduling_info() const { return m_schedulingInfo; }
    uint64 get_view_count() const { return m_viewCount; }

    void set_texture(rhi::TextureHandle textureHandle);
    void set_buffer(rhi::BufferHandle bufferHanlde);
    void set_from_resource(Resource& other);

private:
    ResourceName m_name;
    std::unique_ptr<Buffer> m_buffer = nullptr;
    std::unique_ptr<Texture> m_texture = nullptr;
    ResourceSchedulingInfo m_schedulingInfo;
    uint64 m_viewCount;
};

}