#pragma once

#include "handles/command_buffer.h"
#include "handles/texture_view.h"
#include "rhi/fwd.h"
#include "rhi/resources/enums.h"
#include "core/types.h"
#include <vector>
#include <string>

namespace fe::renderer
{

class DeletionQueue;
class ShaderManager;

class ImGuiRenderer
{
public:
    ImGuiRenderer(DeletionQueue* deletionQueue, ShaderManager* shaderManager);
    ~ImGuiRenderer();

    void begin_frame();
    void create_pipeline();
    void draw(CommandBufferRef cmd);
    void create_font_texture(CommandBufferRef cmd);

    void set_render_target_format(Format format)
    {
        m_renderTargetFormat = format;
    }

    void set_viewport_texture_descriptor(uint32 desciptor)
    {
        m_viewportTextureDescriptor = desciptor;
    }

    bool is_font_texture_created() const
    {
        return m_fontTexture && m_fontTextureView;
    }

private:
    DeletionQueue* m_deletionQueue = nullptr;
    ShaderManager* m_shaderManager = nullptr;

    PipelineHandle m_pipeline;
    Format m_renderTargetFormat = Format::UNDEFINED;
    uint32 m_viewportTextureDescriptor = ~0u;

    std::vector<BufferHandle> m_vertexBuffers;
    std::vector<BufferHandle> m_indexBuffers;
    TextureHandle m_fontTexture;
    TextureViewHandle m_fontTextureView;

    BufferRef get_vertex_buffer(uint32 desiredSize);
    BufferRef get_index_buffer(uint32 desiredSize);
    BufferHandle create_vertex_buffer(uint32 desiredSize);
    BufferHandle create_index_buffer(uint32 desiredSize);
};

}