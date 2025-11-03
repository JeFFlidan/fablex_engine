#pragma once

#include "common.h"
#include "render_context.h"
#include "core/types.h"
#include "rhi/fwd.h"
#include <optional>

namespace fe::renderer::rg
{

struct TextureMetadata;

enum class TextureReadContext : uint8
{
    ANY_SHADER,
    FRAGMENT,
    NON_FRAGMENT
};

class ResourceScheduler
{
public:
    static void init(const RenderContext* renderContext);

    static void create_render_target(
        RenderPassName renderPassName,
        const TextureMetadata* textureMetadata = nullptr,
        std::optional<ResourceName> customResourceName = std::nullopt
    );
    
    static void create_depth_stencil(
        RenderPassName renderPassName,
        const TextureMetadata* textureMetadata = nullptr,
        std::optional<ResourceName> customResourceName = std::nullopt
    );

    static void create_storage_texture(
        RenderPassName renderPassName,
        const TextureMetadata* textureMetadata = nullptr,
        std::optional<ResourceName> customResourceName = std::nullopt
    );
    
    static void write_to_back_buffer(RenderPassName renderPassName);
    static void read_texture(RenderPassName renderPassName, ResourceName resourceName);
    static void read_previous_texture(
        RenderPassName renderPassName,
        const TextureMetadata* textureMetadata = nullptr,
        std::optional<ResourceName> customResourceName = std::nullopt
    );

    static void use_ray_tracing(RenderPassName renderPassName);

private:
    inline static const RenderContext* s_renderContext = nullptr;

    static void queue_resource_allocation(
        RenderPassName renderPassName,
        const TextureMetadata* textureMetadata,
        std::optional<ResourceName> customResourceName,
        Format format,
        ResourceUsage mainUsage,
        ResourceLayout initialLayout
    );

    static void fill_info_from_metadata(
        ResourceUsage mainTextureUsage,
        const TextureMetadata* inMetadata,
        TextureCreateInfo& outInfo
    );
    
    static void add_render_graph_read_dependency(
        RenderPassName renderPassName,
        ResourceName resourceName,
        uint32 mipCount
    );
    
    static void add_render_graph_write_dependency(
        RenderPassName renderPassName,
        ResourceName resourceName,
        uint32 mipCount
    );

    static void update_view_infos(
        RenderPassName renderPassName,
        ResourceSchedulingInfo& schedulingInfo,
        ResourceLayout layout,
        uint32 mipCount
    );
};

}