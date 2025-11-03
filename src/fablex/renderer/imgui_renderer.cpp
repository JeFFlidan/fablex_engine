#include "imgui_renderer.h"
#include "deletion_queue.h"
#include "shader_manager.h"
#include "globals.h"
#include "render_graph/resource_metadata.h"
#include "imgui_interop_editor.h"
#include "accessor.h"
#include "utils.h"

#include "rhi/rhi.h"
#include "rhi/utils.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/texture.h"
#include "rhi/resources/texture_view.h"
#include "rhi/resources/viewport.h"
#include "rhi/resources/graphics_pipeline_info.h"
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

}

void ImGuiRenderer::begin_frame()
{

}

void ImGuiRenderer::create_pipeline()
{
    rg::ShaderMetadata vertShaderMetadata;
    vertShaderMetadata.filePath = VERT_SHADER_PATH;
    vertShaderMetadata.type = ShaderType::VERTEX;
    m_shaderManager->request_shader_loading(vertShaderMetadata);

    rg::ShaderMetadata fragShaderMetadata;
    fragShaderMetadata.filePath = FRAG_SHADER_PATH;
    fragShaderMetadata.type = ShaderType::FRAGMENT;
    m_shaderManager->request_shader_loading(fragShaderMetadata);

    GraphicsPipelineCreateInfo info;
    
    VertexBindingDescription& binding = info.bindingDescriptions.emplace_back();
    binding.binding = 0;
    binding.stride = sizeof(ImDrawVert);

    info.attributeDescriptions.resize(3);
    info.attributeDescriptions[0].location = 0;
    info.attributeDescriptions[0].binding = binding.binding;
    info.attributeDescriptions[0].format = Format::R32G32_SFLOAT;
    info.attributeDescriptions[0].offset = offsetof(ImDrawVert, pos);
    info.attributeDescriptions[1].location = 1;
    info.attributeDescriptions[1].binding = binding.binding;
    info.attributeDescriptions[1].format = Format::R32G32_SFLOAT;
    info.attributeDescriptions[1].offset = offsetof(ImDrawVert, uv);
    info.attributeDescriptions[2].location = 2;
    info.attributeDescriptions[2].binding = binding.binding;
    info.attributeDescriptions[2].format = Format::R8G8B8A8_UNORM;
    info.attributeDescriptions[2].offset = offsetof(ImDrawVert, col);

    info.assemblyState.topologyType = TopologyType::TRIANGLE;

    info.rasterizationState.lineWidth = 1.0f;
    info.rasterizationState.cullMode = CullMode::NONE;
    info.rasterizationState.polygonMode = PolygonMode::FILL;
    info.rasterizationState.frontFace = FrontFace::COUNTER_CLOCKWISE;
    info.rasterizationState.isBiasEnabled = false;

    info.multisampleState.isEnabled = false;
    info.multisampleState.sampleCount = SampleCount::BIT_1;

    ColorBlendAttachmentState& blendAttach = info.colorBlendState.colorBlendAttachments.emplace_back();
    blendAttach.isBlendEnabled = true;
    blendAttach.srcColorBlendFactor = BlendFactor::SRC_ALPHA;
    blendAttach.dstColorBlendFactor = BlendFactor::ONE_MINUS_SRC_ALPHA;
    blendAttach.colorBlendOp = BlendOp::ADD;
    blendAttach.srcAlphaBlendFactor = BlendFactor::ONE;
    blendAttach.dstAlphaBlendFactor = BlendFactor::ONE_MINUS_SRC_ALPHA;
    blendAttach.alphaBlendOp = BlendOp::ADD;
    blendAttach.colorWriteMask = 0x00000001 | 0x00000002 | 0x00000004 | 0x00000008;

    m_shaderManager->wait_shaders_loading();
    info.shaderStages.push_back(m_shaderManager->get_shader(vertShaderMetadata));
    info.shaderStages.push_back(m_shaderManager->get_shader(fragShaderMetadata));

    info.colorAttachmentFormats.push_back(m_renderTargetFormat);

    m_pipeline.init(info);
    m_pipeline.set_name(PIPELINE_NAME);
}

void ImGuiRenderer::draw(CommandBufferRef cmd)
{
    FE_CHECK(cmd);

    ImDrawData* drawData = ImGui::GetDrawData();

    int fbWidth = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    int fbHeight = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
    if (fbWidth <= 0 || fbHeight <= 0)
        return; 

    uint32 vertexSize = rhi::align_to(uint32(drawData->TotalVtxCount * sizeof(ImDrawVert)), BUFFER_ALIGNMENT);
    uint32 indexSize = rhi::align_to(uint32(drawData->TotalIdxCount * sizeof(ImDrawIdx)), BUFFER_ALIGNMENT);

    BufferRef vertexBuffer = get_vertex_buffer(vertexSize);
    BufferRef indexBuffer = get_index_buffer(indexSize);

    FE_CHECK(vertexBuffer);
    FE_CHECK(indexBuffer);

    ImDrawVert* vertexDst = static_cast<ImDrawVert*>(vertexBuffer->mappedData);
    ImDrawIdx* indexDst = static_cast<ImDrawIdx*>(indexBuffer->mappedData);

    FE_CHECK(vertexDst);
    FE_CHECK(indexDst);

    for (uint32 i = 0; i != drawData->CmdListsCount; ++i)
    {
        const ImDrawList* drawList = drawData->CmdLists[i];
        memcpy(vertexDst, drawList->VtxBuffer.Data, drawList->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(indexDst, drawList->IdxBuffer.Data, drawList->IdxBuffer.Size * sizeof(ImDrawIdx));
        vertexDst += drawList->VtxBuffer.Size;
        indexDst += drawList->IdxBuffer.Size;
    }

    ImGuiPushConstants pushConstants;
    pushConstants.sampler = Accessor::get_sampler_linear_clamp();
    pushConstants.scale.x = 2.0f / drawData->DisplaySize.x;
    pushConstants.scale.y = 2.0f / drawData->DisplaySize.y;
    pushConstants.translate.x = -1.0f - drawData->DisplayPos.x * pushConstants.scale.x;
    pushConstants.translate.y = -1.0f - drawData->DisplayPos.y * pushConstants.scale.y;

    Float2 clipOff = Float2(drawData->DisplayPos.x, drawData->DisplayPos.y);
    Float2 clipScale = Float2(drawData->FramebufferScale.x, drawData->FramebufferScale.y);

    Viewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = fbWidth;
    viewport.height = -fbHeight;
    viewport.minDepth = 0.0;
    viewport.maxDepth = 1.0;

    cmd.set_viewports({ viewport });

    uint32 globalVertexOffset = 0;
    uint32 globalIndexOffset = 0;

    cmd.bind_pipeline(m_pipeline);
    cmd.bind_vertex_buffer(vertexBuffer);
    cmd.bind_index_buffer(indexBuffer, 0);

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

            Scissor scissor;
            scissor.left = (int32)clipMin.x;
            scissor.top = (int32)clipMin.y;
            scissor.right = (int32)clipMax.x;
            scissor.bottom = (int32)clipMax.y;

            if (imCmd->GetTexID() == VIEWPORT_IMAGE_DESCRIPTOR)
                pushConstants.texture = m_viewportTextureDescriptor;
            else
                pushConstants.texture = imCmd->GetTexID();

            cmd.push_constants(m_pipeline, &pushConstants);
            cmd.set_scissors({ scissor });
            cmd.draw_indexed(                
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

    Scissor scissor;
    scissor.left = 0;
    scissor.top = 0;
    scissor.right = fbWidth;
    scissor.bottom = fbHeight;

    cmd.set_scissors({ scissor });
}

void ImGuiRenderer::create_font_texture(CommandBufferRef cmd)
{
    if (m_fontTextureView && m_fontTexture)
        return;

    ImGuiIO& io = ImGui::GetIO();
    
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    const uint32 pixelsByteSize = width * height * 4;

    m_fontTexture.init(
        TextureCreateInfo
        {            
            .width = (uint32)width,
            .height = (uint32)height,
            .format = Format::R8G8B8A8_UNORM,
            .textureUsage = ResourceUsage::TRANSFER_DST | ResourceUsage::SAMPLED_TEXTURE,
            .memoryUsage = MemoryUsage::GPU,
            .samplesCount = SampleCount::BIT_1,
            .dimension = TextureDimension::TEXTURE2D,
        }
    );

    m_fontTexture.set_name(FONT_TEXTURE_NAME);

    BufferHandle uploadBuffer(
        BufferCreateInfo
        {            
            .size = pixelsByteSize, // because 4 channels
            .bufferUsage = ResourceUsage::TRANSFER_SRC,
            .memoryUsage = MemoryUsage::CPU,
            .initData = pixels,
            .initDataSize = pixelsByteSize,
        }
    );

    TextureInitInfo textureInitInfo;
    textureInitInfo.buffer = uploadBuffer;
    textureInitInfo.mipMaps.push_back(MipMap{0, 0});
    cmd.init_texture(m_fontTexture, textureInitInfo);    
    
    m_fontTextureView.init(
        TextureViewCreateInfo
        {            
            .texture = m_fontTexture,
            .type = ViewType::SRV,
        }
    );

    m_fontTextureView.set_name(FONT_TEXTURE_VIEW_NAME);

    io.Fonts->SetTexID(m_fontTextureView->descriptorIndex);

    m_deletionQueue->add(uploadBuffer);
}

BufferRef ImGuiRenderer::get_vertex_buffer(uint32 desiredSize)
{
    if (m_vertexBuffers.size() < g_frameIndex + 1)
        m_vertexBuffers.push_back(create_vertex_buffer(desiredSize));

    if (m_vertexBuffers.at(g_frameIndex)->size < desiredSize)
    {
        m_deletionQueue->add(m_vertexBuffers[g_frameIndex]);
        m_vertexBuffers[g_frameIndex] = create_vertex_buffer(desiredSize);
    }

    return m_vertexBuffers[g_frameIndex];
}

BufferRef ImGuiRenderer::get_index_buffer(uint32 desiredSize)
{
    if (m_indexBuffers.size() < g_frameIndex + 1)
        m_indexBuffers.push_back(create_index_buffer(desiredSize));

    if (m_indexBuffers.at(g_frameIndex)->size < desiredSize)
    {
        m_deletionQueue->add(m_indexBuffers[g_frameIndex]);
        m_indexBuffers[g_frameIndex] = create_index_buffer(desiredSize);
    }

    return m_indexBuffers[g_frameIndex];
}

BufferHandle ImGuiRenderer::create_vertex_buffer(uint32 desiredSize)
{
    BufferHandle buffer(
        BufferCreateInfo
        {            
            .size = desiredSize ? desiredSize : BUFFER_ALIGNMENT,
            .bufferUsage = ResourceUsage::VERTEX_BUFFER,
            .memoryUsage = MemoryUsage::CPU_TO_GPU,
        }
    );

    Utils::set_debug_name(buffer, VERTEX_BUFFER_NAME);

    return buffer;
}

BufferHandle ImGuiRenderer::create_index_buffer(uint32 desiredSize)
{
    BufferHandle buffer(
        BufferCreateInfo
        {            
            .size = desiredSize ? desiredSize : BUFFER_ALIGNMENT,
            .bufferUsage = ResourceUsage::INDEX_BUFFER,
            .memoryUsage = MemoryUsage::CPU_TO_GPU,
        }
    );

    Utils::set_debug_name(buffer, INDEX_BUFFER_NAME);

    return buffer;
}

}