#include "render_graph_metadata.h"
#include "shader_manager.h"
#include "render_context.h"
#include "core/json_serialization.h"
#include "rhi/json_serialization.h"
#include "json_serialization.h"
#include "rhi/utils.h"
#include "core/file_system/file_system.h"

namespace fe::renderer
{

constexpr const char* g_renderTexturesKey = "RenderTextures";
constexpr const char* g_renderPassesKey = "RenderPasses";
constexpr const char* g_formatKey = "Format";
constexpr const char* g_isTransferDstKey = "IsTransferDst";
constexpr const char* g_useMipsKey = "UseMips";
constexpr const char* g_layerCountKey = "LayerCount";
constexpr const char* g_sampleCountKey = "SampleCount";
constexpr const char* g_inputTexturesKey = "InputTextures";
constexpr const char* g_renderTargetTexturesKey = "RenderTargets";
constexpr const char* g_outputStorageTexturesKey = "OutputStorageTextures";
constexpr const char* g_storeOpKey = "StoreOp";
constexpr const char* g_loadOpKey = "LoadOp";
constexpr const char* g_colorClearValueKey = "ColorClearValue";
constexpr const char* g_depthStencilClearValueKay = "DepthStencilClearValue";
constexpr const char* g_pipelineKey = "Pipeline";
constexpr const char* g_shadersKey = "Shaders";
constexpr const char* g_definesKey = "Defines";
constexpr const char* g_hitGroupTypeKey = "HitGroupType";
constexpr const char* g_entryPointKey = "EntryPoint";
constexpr const char* g_typeKey = "Type";
constexpr const char* g_pathKey = "Path";

RenderGraphMetadata::RenderGraphMetadata(const RenderContext* renderContext)
{
    FE_CHECK(renderContext);
    m_shaderManager = renderContext->get_shader_manager();
    FE_CHECK(m_shaderManager);
}

void RenderGraphMetadata::deserialize(const std::string& path)
{
    m_renderPassMetadataByName.clear();
    m_pipelineMetadataByName.clear();
    m_renderTextureMetadataByName.clear();

    FE_LOG(LogRenderer, INFO, "Starting deserializing render graph metadata '{}'", path);

    std::string jsonStr;
    FileSystem::read(FileSystem::get_absolute_path(path), jsonStr);
    nlohmann::json json = nlohmann::json::parse(jsonStr);

    // const std::vector<nlohmann::json>& pushConstantsMetadataJsons = json[g_pushConstantsKey];

    if (json.contains(g_renderTexturesKey))
    {
        const std::vector<nlohmann::json>& renderTextureMetadataJsons = json[g_renderTexturesKey];
        m_renderTextureMetadataByName.reserve(renderTextureMetadataJsons.size());

        for (const nlohmann::json& textureMetadataJson : renderTextureMetadataJsons)
        {
            ResourceName textureName = textureMetadataJson[g_nameKey];
            TextureMetadata& textureMetadata = m_renderTextureMetadataByName[textureName];
            textureMetadata.textureName = textureName;

            if (textureMetadataJson.contains(g_formatKey))
                textureMetadata.format = textureMetadataJson[g_formatKey];

            if (textureMetadataJson.contains(g_isTransferDstKey))
                textureMetadata.isTransferDst = textureMetadataJson[g_isTransferDstKey];

            if (textureMetadataJson.contains(g_useMipsKey))
                textureMetadata.useMips = textureMetadataJson[g_useMipsKey];

            if (textureMetadataJson.contains(g_layerCountKey))
                textureMetadata.layerCount = textureMetadataJson[g_layerCountKey];

            if (textureMetadataJson.contains(g_sampleCountKey))
            {
                uint32 sampleCount = textureMetadataJson[g_sampleCountKey];
                switch (sampleCount)
                {
                case 1: textureMetadata.sampleCount = rhi::SampleCount::BIT_1; break;
                case 2: textureMetadata.sampleCount = rhi::SampleCount::BIT_2; break;
                case 4: textureMetadata.sampleCount = rhi::SampleCount::BIT_4; break;
                case 8: textureMetadata.sampleCount = rhi::SampleCount::BIT_8; break;
                case 16: textureMetadata.sampleCount = rhi::SampleCount::BIT_16; break;
                case 32: textureMetadata.sampleCount = rhi::SampleCount::BIT_32; break;
                case 64: textureMetadata.sampleCount = rhi::SampleCount::BIT_64; break;
                default: textureMetadata.sampleCount = rhi::SampleCount::UNDEFINED; break;
                }
            }
        }
    }

    const std::vector<nlohmann::json>& renderPassMetadataJsons = json[g_renderPassesKey];
    m_renderPassMetadataByName.reserve(renderPassMetadataJsons.size());
    m_pipelineMetadataByName.reserve(m_renderPassMetadataByName.size());

    for (const nlohmann::json& renderPassMetadataJson : renderPassMetadataJsons)
    {
        RenderPassName renderPassName = renderPassMetadataJson[g_nameKey];
        RenderPassMetadata& renderPassMetadata = m_renderPassMetadataByName[renderPassName];

        renderPassMetadata.renderPassName = renderPassName;

        if (renderPassMetadataJson.contains(g_typeKey))
            renderPassMetadata.type = renderPassMetadataJson[g_typeKey];

        if (renderPassMetadataJson.contains(g_inputTexturesKey))
            renderPassMetadata.inputTextureNames = renderPassMetadataJson[g_inputTexturesKey];

        if (renderPassMetadataJson.contains(g_outputStorageTexturesKey))
            renderPassMetadata.outputStorageTextureNames = renderPassMetadataJson[g_outputStorageTexturesKey];

        if (renderPassMetadataJson.contains(g_renderTargetTexturesKey))
        {
            const std::vector<nlohmann::json>& renderTargetMetadataJsons = renderPassMetadataJson[g_renderTargetTexturesKey];
            renderPassMetadata.renderTargetsMetadata.reserve(renderTargetMetadataJsons.size());

            for (const nlohmann::json& renderTargetMetadataJson : renderTargetMetadataJsons)
            {
                RenderTargetMetadata& renderTargetMetadata = renderPassMetadata.renderTargetsMetadata.emplace_back();

                // Make render target name unnecessary because swap chain pass can describe render target without metadata
                if (renderTargetMetadataJson.contains(g_nameKey))
                {
                    renderTargetMetadata.textureName = renderTargetMetadataJson[g_nameKey];
                    const TextureMetadata* textureMetadata = get_texture_metadata(renderTargetMetadata.textureName);
                    FE_CHECK(textureMetadata);
                    renderTargetMetadata.format = textureMetadata->format;
                }

                if (renderTargetMetadataJson.contains(g_storeOpKey))
                    renderTargetMetadata.storeOp = renderTargetMetadataJson[g_storeOpKey];

                if (renderTargetMetadataJson.contains(g_loadOpKey))
                    renderTargetMetadata.loadOp = renderTargetMetadataJson[g_loadOpKey];

                if (renderTargetMetadataJson.contains(g_colorClearValueKey))
                    renderTargetMetadata.clearValues.color = renderTargetMetadataJson[g_colorClearValueKey];

                if (renderTargetMetadataJson.contains(g_depthStencilClearValueKay))
                {
                    std::array<float, 2> values = renderTargetMetadataJson[g_depthStencilClearValueKay];
                    renderTargetMetadata.clearValues.depthStencil.depth = values[0];
                    renderTargetMetadata.clearValues.depthStencil.stencil = (uint32)values[1];
                }
            }

        }
        
        if (!renderPassMetadataJson.contains(g_pipelineKey))
        {
            FE_LOG(LogRenderer, ERROR, "No pipeline metadata for render pass {}", renderPassMetadata.renderPassName);
            continue;
        }

        const nlohmann::json& pipelineMetadataJson = renderPassMetadataJson[g_pipelineKey];
        
        PipelineName pipelineName = renderPassName;
        if (pipelineMetadataJson.contains(g_nameKey))
            pipelineName = pipelineMetadataJson[g_nameKey];

        renderPassMetadata.pipelineName = pipelineName;

        PipelineMetadata& pipelineMetadata = m_pipelineMetadataByName[pipelineName];
        pipelineMetadata.pipelineName = pipelineName;

        if (!pipelineMetadataJson.contains(g_shadersKey))
        {
            FE_LOG(LogRenderer, ERROR, "No 'Shaders' key in render pass {}", renderPassName);
            continue;
        }

        const std::vector<nlohmann::json>& shaderJsons = pipelineMetadataJson[g_shadersKey];
        pipelineMetadata.shadersMetadata.reserve(shaderJsons.size());

        for (const nlohmann::json& shaderMetadataJson : shaderJsons)
        {
            ShaderMetadata& shaderMetadata = pipelineMetadata.shadersMetadata.emplace_back();
            shaderMetadata.type = shaderMetadataJson[g_typeKey];
            shaderMetadata.filePath = shaderMetadataJson[g_pathKey];

            if (shaderMetadataJson.contains(g_definesKey))
                shaderMetadata.defines = shaderMetadataJson[g_definesKey];

            if (shaderMetadataJson.contains(g_hitGroupTypeKey))
                shaderMetadata.hitGroupType = shaderMetadataJson[g_hitGroupTypeKey];

            if (shaderMetadataJson.contains(g_entryPointKey))
                shaderMetadata.entryPoint = shaderMetadataJson[g_entryPointKey];

            m_shaderManager->request_shader_loading(shaderMetadata);
        }

        for (const RenderTargetMetadata& renderTargetMetadata : renderPassMetadata.renderTargetsMetadata)
        {
            if (rhi::is_depth_stencil_format(renderTargetMetadata.format))
                pipelineMetadata.depthStencilFormat = renderTargetMetadata.format;
            else
                pipelineMetadata.colorAttachmentFormats.push_back(renderTargetMetadata.format);
        }
    }

    FE_LOG(LogRenderer, INFO, "Deserializing render graph metadata '{}' completed", path);
}

const TextureMetadata* RenderGraphMetadata::get_texture_metadata(ResourceName textureName) const
{
    auto it = m_renderTextureMetadataByName.find(textureName);
    if (it == m_renderTextureMetadataByName.end())
        return nullptr;

    return &it->second;
}

const RenderPassMetadata* RenderGraphMetadata::get_render_pass_metadata(RenderPassName renderPassName) const
{
    auto it = m_renderPassMetadataByName.find(renderPassName);
    if (it == m_renderPassMetadataByName.end())
        return nullptr;

    return &it->second;
}

const PipelineMetadata* RenderGraphMetadata::get_pipeline_metadata(PipelineName pipelineName) const
{
    auto it = m_pipelineMetadataByName.find(pipelineName);
    if (it == m_pipelineMetadataByName.end())
        return nullptr;

    return &it->second;
}

}