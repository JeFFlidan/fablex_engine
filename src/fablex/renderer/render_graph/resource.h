#pragma once

#include "common.h"
#include "buffer.h"
#include "texture.h"
#include "resource_scheduling_info.h"
#include <memory>

namespace fe::renderer::rg
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

    bool is_valid() const { return (m_texture && m_texture->handle()) || (m_buffer && m_buffer->handle()); }
    bool is_buffer() const { return (m_buffer && m_buffer->handle()); }
    bool is_texture() const { return (m_texture && m_texture->handle()); }
    
    ResourceName name() const { return m_name; }
    Buffer& buffer() { return *m_buffer; }
    const Buffer& buffer() const { return *m_buffer; }
    Texture& texture() { return *m_texture;}
    const Texture& texture() const { return *m_texture; }
    ResourceSchedulingInfo& scheduling_info() { return m_schedulingInfo; }
    const ResourceSchedulingInfo& scheduling_info() const { return m_schedulingInfo; }
    uint64 view_count() const { return m_viewCount; }

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