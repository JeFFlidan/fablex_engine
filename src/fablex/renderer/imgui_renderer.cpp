#include "imgui_renderer.h"
#include "deletion_queue.h"
#include "shader_manager.h"
#include "globals.h"
#include "resource_metadata.h"
#include "imgui_interop_editor.h"
#include "utils.h"

#include "rhi/rhi.h"
#include "rhi/utils.h"
#include "shaders/shader_interop_renderer.h"

#include "imgui.h"

namespace fe::renderer
{

constexpr uint32 BUFFER_ALIGNMENT = 256;
constexpr const char* VERT_SHADER_PATH = "imgui_vs.hlsl";
constexpr const char* FRAG_SHADER_PATH = "imgui_fs.hlsl";
constexpr const char* VERTEX_BUFFER_NAME = "ImGuiVertexBuffer";
constexpr const char* INDEX_BUFFER_NAME = "ImGuiIndexBuffer";
constexpr const char* FONT_TEXTURE_NAME = "ImGuiFontTexture";
constexpr const char* FONT_TEXTURE_VIEW_NAME = "ImGuiFontTextureView";
constexpr const char* PIPELINE_NAME = "ImGuiPipeline";

ImGuiRenderer::ImGuiRenderer(DeletionQueue* deletionQueue, ShaderManager* shaderManager)
    : m_deletionQueue(deletionQueue), m_shaderManager(shaderManager)
{
    FE_CHECK(m_deletionQueue);
}

ImGuiRenderer::~ImGuiRenderer()
{
    rhi::destroy_pipeline(m_pipeline);
    
    for (rhi::Buffer* buffer : m_vertexBuffers)
        rhi::destroy_buffer(buffer);
    for (rhi::Buffer* buffer : m_indexBuffers)
        rhi ::destroy_buffer(buffer);

    rhi::destroy_texture_view(m_fontTextureView);
    rhi::destroy_texture(m_fontTexture);
}

void ImGuiRenderer::begin_frame()
{

}

void ImGuiRenderer::create_pipeline()
{
    ShaderMetadata vertShaderMetadata;
    vertShaderMetadata.filePath = VERT_SHADER_PATH;
    vertShaderMetadata.type = rhi::ShaderType::VERTEX;
    m_shaderManager->request_shader_loading(vertShaderMetadata);

    ShaderMetadata fragShaderMetadata;
    fragShaderMetadata.filePath = FRAG_SHADER_PATH;
    fragShaderMetadata.type = rhi::ShaderType::FRAGMENT;
    m_shaderManager->request_shader_loading(fragShaderMetadata);

    rhi::GraphicsPipelineInfo info;
    
    rhi::VertexBindingDescription& binding = info.bindingDescriptions.emplace_back();
    binding.binding = 0;
    binding.stride = sizeof(ImDrawVert);

    info.attributeDescriptions.resize(3);
    info.attributeDescriptions[0].location = 0;
    info.attributeDescriptions[0].binding = binding.binding;
    info.attributeDescriptions[0].format = rhi::Format::R32G32_SFLOAT;
    info.attributeDescriptions[0].offset = offsetof(ImDrawVert, pos);
    info.attributeDescriptions[1].location = 1;
    info.attributeDescriptions[1].binding = binding.binding;
    info.attributeDescriptions[1].format = rhi::Format::R32G32_SFLOAT;
    info.attributeDescriptions[1].offset = offsetof(ImDrawVert, uv);
    info.attributeDescriptions[2].location = 2;
    info.attributeDescriptions[2].binding = binding.binding;
    info.attributeDescriptions[2].format = rhi::Format::R8G8B8A8_UNORM;
    info.attributeDescriptions[2].offset = offsetof(ImDrawVert, col);

    info.assemblyState.topologyType = rhi::TopologyType::TRIANGLE;

    info.rasterizationState.lineWidth = 1.0f;
    info.rasterizationState.cullMode = rhi::CullMode::NONE;
    info.rasterizationState.polygonMode = rhi::PolygonMode::FILL;
    info.rasterizationState.frontFace = rhi::FrontFace::COUNTER_CLOCKWISE;
    info.rasterizationState.isBiasEnabled = false;

    info.multisampleState.isEnabled = false;
    info.multisampleState.sampleCount = rhi::SampleCount::BIT_1;

    rhi::ColorBlendAttachmentState& blendAttach = info.colorBlendState.colorBlendAttachments.emplace_back();
    blendAttach.isBlendEnabled = true;
    blendAttach.srcColorBlendFactor = rhi::BlendFactor::SRC_ALPHA;
    blendAttach.dstColorBlendFactor = rhi::BlendFactor::ONE_MINUS_SRC_ALPHA;
    blendAttach.colorBlendOp = rhi::BlendOp::ADD;
    blendAttach.srcAlphaBlendFactor = rhi::BlendFactor::ONE;
    blendAttach.dstAlphaBlendFactor = rhi::BlendFactor::ONE_MINUS_SRC_ALPHA;
    blendAttach.alphaBlendOp = rhi::BlendOp::ADD;
    blendAttach.colorWriteMask = 0x00000001 | 0x00000002 | 0x00000004 | 0x00000008;

    m_shaderManager->wait_shaders_loading();
    info.shaderStages.push_back(m_shaderManager->get_shader(vertShaderMetadata));
    info.shaderStages.push_back(m_shaderManager->get_shader(fragShaderMetadata));

    info.colorAttachmentFormats.push_back(m_renderTargetFormat);

    rhi::create_graphics_pipeline(&m_pipeline, &info);

    rhi::set_name(m_pipeline, PIPELINE_NAME);
}

void ImGuiRenderer::draw(rhi::CommandBuffer* cmd)
{
    FE_CHECK(cmd);

    ImDrawData* drawData = ImGui::GetDrawData();

    int fbWidth = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    int fbHeight = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
    if (fbWidth <= 0 || fbHeight <= 0)
        return; 

    uint32 vertexSize = rhi::align_to(uint32(drawData->TotalVtxCount * sizeof(ImDrawVert)), BUFFER_ALIGNMENT);
    uint32 indexSize = rhi::align_to(uint32(drawData->TotalIdxCount * sizeof(ImDrawIdx)), BUFFER_ALIGNMENT);

    rhi::Buffer* vertexBuffer = get_vertex_buffer(vertexSize);
    rhi::Buffer* indexBuffer = get_index_buffer(indexSize);

    ImDrawVert* vertexDst = static_cast<ImDrawVert*>(vertexBuffer->mappedData);
    ImDrawIdx* indexDst = static_cast<ImDrawIdx*>(indexBuffer->mappedData);

    for (uint32 i = 0; i != drawData->CmdListsCount; ++i)
    {
        const ImDrawList* drawList = drawData->CmdLists[i];
        memcpy(vertexDst, drawList->VtxBuffer.Data, drawList->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(indexDst, drawList->IdxBuffer.Data, drawList->IdxBuffer.Size * sizeof(ImDrawIdx));
        vertexDst += drawList->VtxBuffer.Size;
        indexDst += drawList->IdxBuffer.Size;
    }

    ImGuiPushConstants pushConstants;
    pushConstants.sampler = Utils::get_sampler_linear_clamp();
    pushConstants.scale.x = 2.0f / drawData->DisplaySize.x;
    pushConstants.scale.y = 2.0f / drawData->DisplaySize.y;
    pushConstants.translate.x = -1.0f - drawData->DisplayPos.x * pushConstants.scale.x;
    pushConstants.translate.y = -1.0f - drawData->DisplayPos.y * pushConstants.scale.y;

    Float2 clipOff = Float2(drawData->DisplayPos.x, drawData->DisplayPos.y);
    Float2 clipScale = Float2(drawData->FramebufferScale.x, drawData->FramebufferScale.y);

    rhi::Viewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = fbWidth;
    viewport.height = -fbHeight;
    viewport.minDepth = 0.0;
    viewport.maxDepth = 1.0;

    rhi::set_viewports(cmd, {viewport});

    uint32 globalVertexOffset = 0;
    uint32 globalIndexOffset = 0;

    rhi::bind_pipeline(cmd, m_pipeline);
    rhi::bind_vertex_buffer(cmd, vertexBuffer);
    rhi::bind_index_buffer(cmd, indexBuffer, 0);

    for (uint32 i = 0; i != drawData->CmdListsCount; ++i)
    {
        const ImDrawList* drawList = drawData->CmdLists[i];
        for (uint32 j = 0; j != drawList->CmdBuffer.Size; ++j)
        {
            const ImDrawCmd* imCmd = &drawList->CmdBuffer[j];
            
            Float2 clipMin((imCmd->ClipRect.x - clipOff.x) * clipScale.x, (imCmd->ClipRect.y - clipOff.y) * clipScale.y);
            Float2 clipMax((imCmd->ClipRect.z - clipOff.x) * clipScale.x, (imCmd->ClipRect.w - clipOff.y) * clipScale.y);

            if (clipMin.x < 0.0f) { clipMin.x = 0.0f; }
            if (clipMin.y < 0.0f) { clipMin.y = 0.0f; }
            if (clipMax.x > fbWidth) { clipMax.x = (float)fbWidth; }
            if (clipMax.y > fbHeight) { clipMax.y = (float)fbHeight; }
            if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                continue;

            rhi::Scissor scissor;
            scissor.left = (int32)clipMin.x;
            scissor.top = (int32)clipMin.y;
            scissor.right = (int32)clipMax.x;
            scissor.bottom = (int32)clipMax.y;

            if (imCmd->GetTexID() == VIEWPORT_IMAGE_DESCRIPTOR)
                pushConstants.texture = m_viewportTextureDescriptor;
            else
                pushConstants.texture = imCmd->GetTexID();

            rhi::push_constants(cmd, m_pipeline, &pushConstants);
            rhi::set_scissors(cmd, {scissor});
            rhi::draw_indexed(
                cmd, 
                imCmd->ElemCount, 
                1, 
                imCmd->IdxOffset + globalIndexOffset, 
                imCmd->VtxOffset + globalVertexOffset, 
                0
            );
        }

        globalVertexOffset += drawList->VtxBuffer.Size;
        globalIndexOffset += drawList->IdxBuffer.Size;
    }

    rhi::Scissor scissor;
    scissor.left = 0;
    scissor.top = 0;
    scissor.right = fbWidth;
    scissor.bottom = fbHeight;

    rhi::set_scissors(cmd, {scissor});
}

void ImGuiRenderer::create_font_texture(rhi::CommandBuffer* cmd)
{
    if (m_fontTextureView && m_fontTexture)
        return;

    ImGuiIO& io = ImGui::GetIO();
    
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    const uint32 pixelsByteSize = width * height * 4;

    rhi::TextureInfo textureInfo;
    textureInfo.width = width;
    textureInfo.height = height;
    textureInfo.format = rhi::Format::R8G8B8A8_UNORM;
    textureInfo.memoryUsage = rhi::MemoryUsage::GPU;
    textureInfo.textureUsage = rhi::ResourceUsage::TRANSFER_DST | rhi::ResourceUsage::SAMPLED_TEXTURE;
    textureInfo.samplesCount = rhi::SampleCount::BIT_1;
    textureInfo.dimension = rhi::TextureDimension::TEXTURE2D;
    rhi::create_texture(&m_fontTexture, &textureInfo);

    rhi::BufferInfo uploadBufferInfo;
    uploadBufferInfo.size = pixelsByteSize; // because 4 channels
    uploadBufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC;
    uploadBufferInfo.memoryUsage = rhi::MemoryUsage::CPU;
    uploadBufferInfo.initData = pixels;
    uploadBufferInfo.initDataSize = pixelsByteSize;

    rhi::TextureInitInfo textureInitInfo;
    rhi::create_buffer(&textureInitInfo.buffer, &uploadBufferInfo);
    textureInitInfo.mipMaps.push_back(rhi::MipMap{0, 0});
    rhi::init_texture(cmd, m_fontTexture, &textureInitInfo);

    rhi::TextureViewInfo textureViewInfo;
    textureViewInfo.type = rhi::ViewType::SRV;
    rhi::create_texture_view(&m_fontTextureView, &textureViewInfo, m_fontTexture);

    io.Fonts->SetTexID(m_fontTextureView->descriptorIndex);

    rhi::set_name(m_fontTexture, FONT_TEXTURE_NAME);
    rhi::set_name(m_fontTextureView, FONT_TEXTURE_VIEW_NAME);

    rhi::Buffer* uploadBuffer = textureInitInfo.buffer;
    m_deletionQueue->add([uploadBuffer]()
    {
        rhi::destroy_buffer(uploadBuffer);
    });
}

rhi::Buffer* ImGuiRenderer::get_vertex_buffer(uint32 desiredSize)
{
    if (m_vertexBuffers.size() < g_frameIndex + 1)
        m_vertexBuffers.push_back(create_uma_buffer(desiredSize, rhi::ResourceUsage::VERTEX_BUFFER));

    if (m_vertexBuffers.at(g_frameIndex)->size < desiredSize)
    {
        rhi::Buffer* oldBuffer = m_vertexBuffers[g_frameIndex];
        m_deletionQueue->add([oldBuffer]()
        {
            rhi::destroy_buffer(oldBuffer);
        });

        m_vertexBuffers[g_frameIndex] = create_uma_buffer(desiredSize, rhi::ResourceUsage::VERTEX_BUFFER);
    }

    return m_vertexBuffers[g_frameIndex];
}

rhi::Buffer* ImGuiRenderer::get_index_buffer(uint32 desiredSize)
{
    if (m_indexBuffers.size() < g_frameIndex + 1)
        m_indexBuffers.push_back(create_uma_buffer(desiredSize, rhi::ResourceUsage::INDEX_BUFFER));

    if (m_indexBuffers.at(g_frameIndex)->size < desiredSize)
    {
        rhi::Buffer* oldBuffer = m_indexBuffers[g_frameIndex];
        m_deletionQueue->add([oldBuffer]()
        {
            rhi::destroy_buffer(oldBuffer);
        });

        m_indexBuffers[g_frameIndex] = create_uma_buffer(desiredSize, rhi::ResourceUsage::INDEX_BUFFER);
    }

    return m_indexBuffers[g_frameIndex];
}

rhi::Buffer* ImGuiRenderer::create_uma_buffer(uint32 desiredSize, rhi::ResourceUsage usage)
{
    rhi::BufferInfo info;
    info.size = desiredSize ? desiredSize : BUFFER_ALIGNMENT;
    info.memoryUsage = rhi::MemoryUsage::CPU_TO_GPU;
    info.bufferUsage = usage;
    
    rhi::Buffer* buffer;

    rhi::create_buffer(&buffer, &info);

    if (has_flag(info.bufferUsage, rhi::ResourceUsage::VERTEX_BUFFER))
        rhi::set_name(buffer, VERTEX_BUFFER_NAME + std::to_string(g_frameIndex));
    else if (has_flag(info.bufferUsage, rhi::ResourceUsage::INDEX_BUFFER))
        rhi::set_name(buffer, INDEX_BUFFER_NAME + std::to_string(g_frameIndex));

    return buffer;
}

}