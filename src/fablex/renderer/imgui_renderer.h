#pragma once

#include "rhi/fwd.h"
#include "rhi/enums.h"
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
    void draw(rhi::CommandBuffer* cmd);
    void create_font_texture(rhi::CommandBuffer* cmd);

    void set_render_target_format(rhi::Format format)
    {
        m_renderTargetFormat = format;
    }

    bool is_font_texture_created() const
    {
        return m_fontTexture && m_fontTextureView;
    }

private:
    DeletionQueue* m_deletionQueue = nullptr;
    ShaderManager* m_shaderManager = nullptr;

    rhi::Pipeline* m_pipeline = nullptr;
    rhi::Format m_renderTargetFormat = rhi::Format::UNDEFINED;

    std::vector<rhi::Buffer*> m_vertexBuffers;
    std::vector<rhi::Buffer*> m_indexBuffers;
    rhi::Texture* m_fontTexture = nullptr;
    rhi::TextureView* m_fontTextureView = nullptr;

    rhi::Buffer* get_vertex_buffer(uint32 desiredSize);
    rhi::Buffer* get_index_buffer(uint32 desiredSize);
    rhi::Buffer* create_uma_buffer(uint32 desiredSize, rhi::ResourceUsage usage);
};

}