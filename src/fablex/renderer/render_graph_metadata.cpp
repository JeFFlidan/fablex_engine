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

constexpr const char* RENDER_TEXTURES_KEY = "RenderTextures";
constexpr const char* PUSH_CONSTANTS_KEY = "PushConstants";
constexpr const char* RESOURCES_KEY = "Resources";
constexpr const char* PREVIOUS_FRAME_KEY = "PreviousFrame";
constexpr const char* IS_STORAGE_KEY = "IsStorage";
constexpr const char* RENDER_PASSES_KEY = "RenderPasses";
constexpr const char* FORMAT_KEY = "Format";
constexpr const char* IS_TRANSFER_DST_KEY = "IsTransferDst";
constexpr const char* USE_MIPS_KEY = "UseMips";
constexpr const char* CROSS_FRAME_READ_KEY = "CrossFrameRead";
constexpr const char* LAYER_COUNT_KEY = "LayerCount";
constexpr const char* SAMPLE_COUNT_KEY = "SampleCount";
constexpr const char* INPUT_TEXTURES_KEY = "InputTextures";
constexpr const char* RENDER_TARGET_TEXTURES_KEY = "RenderTargets";
constexpr const char* OUTPUT_STORAGE_TEXTURES_KEY = "OutputStorageTextures";
constexpr const char* STORE_OP_KEY = "StoreOp";
constexpr const char* LOAD_OP_KEY = "LoadOp";
constexpr const char* COLOR_CLEAR_VALUE_KEY = "ColorClearValue";
constexpr const char* DEPTH_STENCIL_CLEAR_VALUE_KEY = "DepthStencilClearValue";
constexpr const char* PIPELINE_KEY = "Pipeline";
constexpr const char* SHADERS_KEY = "Shaders";
constexpr const char* DEFINES_KEY = "Defines";
constexpr const char* HIT_GROUP_TYPE_KEY = "HitGroupType";
constexpr const char* ENTRY_POINT_KEY = "EntryPoint";
constexpr const char* TYPE_KEY = "Type";
constexpr const char* PATH_KEY = "Path";

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

    if (json.contains(RENDER_TEXTURES_KEY))
    {
        const std::vector<JSON>& renderTextureMetadataJsons = json[RENDER_TEXTURES_KEY];
        m_renderTextureMetadataByName.reserve(renderTextureMetadataJsons.size());

        for (const nlohmann::json& textureMetadataJson : renderTextureMetadataJsons)
        {
            ResourceName textureName = textureMetadataJson[NAME_KEY];
            TextureMetadata& textureMetadata = m_renderTextureMetadataByName[textureName];
            textureMetadata.textureName = textureName;

            if (textureMetadataJson.contains(FORMAT_KEY))
                textureMetadata.format = textureMetadataJson[FORMAT_KEY];

            if (textureMetadataJson.contains(IS_TRANSFER_DST_KEY))
                textureMetadata.isTransferDst = textureMetadataJson[IS_TRANSFER_DST_KEY];

            if (textureMetadataJson.contains(USE_MIPS_KEY))
                textureMetadata.useMips = textureMetadataJson[USE_MIPS_KEY];

            if (textureMetadataJson.contains(CROSS_FRAME_READ_KEY))
                textureMetadata.crossFrameRead = textureMetadataJson[CROSS_FRAME_READ_KEY];

            if (textureMetadataJson.contains(LAYER_COUNT_KEY))
                textureMetadata.layerCount = textureMetadataJson[LAYER_COUNT_KEY];

            if (textureMetadataJson.contains(SAMPLE_COUNT_KEY))
            {
                uint32 sampleCount = textureMetadataJson[SAMPLE_COUNT_KEY];
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

    if (json.contains(PUSH_CONSTANTS_KEY))
    {
        const std::vector<JSON>& pushConstantsMetadataJsons = json[PUSH_CONSTANTS_KEY];
        m_pushConstantsMetadataByName.reserve(pushConstantsMetadataJsons.size());

        for (const nlohmann::json& pushConstantsMetadataJson : pushConstantsMetadataJsons)
        {
            PushConstantsName pushConstantsName = pushConstantsMetadataJson[NAME_KEY];
            PushConstantsMetadata& pushConstantsMetadata = m_pushConstantsMetadataByName[pushConstantsName];
            pushConstantsMetadata.name = pushConstantsName;

            if (pushConstantsMetadataJson.contains(RESOURCES_KEY))
            {
                const std::vector<JSON>& resourceMetadataJsons = pushConstantsMetadataJson[RESOURCES_KEY];
                pushConstantsMetadata.resourcesMetadata.reserve(resourceMetadataJsons.size());
                
                for (const JSON& resourceMetadataJson : resourceMetadataJsons)
                {
                    auto& resourceMetadata = pushConstantsMetadata.resourcesMetadata.emplace_back();
                    resourceMetadata.name = resourceMetadataJson[NAME_KEY];

                    if (resourceMetadataJson.contains(PREVIOUS_FRAME_KEY))
                        resourceMetadata.previousFrame = resourceMetadataJson[PREVIOUS_FRAME_KEY];

                    if (resourceMetadataJson.contains(IS_STORAGE_KEY))
                        resourceMetadata.isStorage = resourceMetadataJson[IS_STORAGE_KEY];
                }
            }
        }
    }

    const std::vector<JSON>& renderPassMetadataJsons = json[RENDER_PASSES_KEY];
    m_renderPassMetadataByName.reserve(renderPassMetadataJsons.size());
    m_pipelineMetadataByName.reserve(m_renderPassMetadataByName.size());

    for (const nlohmann::json& renderPassMetadataJson : renderPassMetadataJsons)
    {
        RenderPassName renderPassName = renderPassMetadataJson[NAME_KEY];
        RenderPassMetadata& renderPassMetadata = m_renderPassMetadataByName[renderPassName];

        renderPassMetadata.renderPassName = renderPassName;

        if (renderPassMetadataJson.contains(TYPE_KEY))
            renderPassMetadata.type = renderPassMetadataJson[TYPE_KEY];

        if (renderPassMetadataJson.contains(INPUT_TEXTURES_KEY))
            renderPassMetadata.inputTextureNames = renderPassMetadataJson[INPUT_TEXTURES_KEY];

        if (renderPassMetadataJson.contains(OUTPUT_STORAGE_TEXTURES_KEY))
            renderPassMetadata.outputStorageTextureNames = renderPassMetadataJson[OUTPUT_STORAGE_TEXTURES_KEY];

        if (renderPassMetadataJson.contains(RENDER_TARGET_TEXTURES_KEY))
        {
            const std::vector<JSON>& renderTargetMetadataJsons = renderPassMetadataJson[RENDER_TARGET_TEXTURES_KEY];
            renderPassMetadata.renderTargetsMetadata.reserve(renderTargetMetadataJsons.size());

            for (const nlohmann::json& renderTargetMetadataJson : renderTargetMetadataJsons)
            {
                RenderTargetMetadata& renderTargetMetadata = renderPassMetadata.renderTargetsMetadata.emplace_back();

                // Make render target name unnecessary because swap chain pass can describe render target without metadata
                if (renderTargetMetadataJson.contains(NAME_KEY))
                {
                    renderTargetMetadata.textureName = renderTargetMetadataJson[NAME_KEY];
                    const TextureMetadata* textureMetadata = get_texture_metadata(renderTargetMetadata.textureName);
                    FE_CHECK(textureMetadata);
                    renderTargetMetadata.format = textureMetadata->format;
                }

                if (renderTargetMetadataJson.contains(STORE_OP_KEY))
                    renderTargetMetadata.storeOp = renderTargetMetadataJson[STORE_OP_KEY];

                if (renderTargetMetadataJson.contains(LOAD_OP_KEY))
                    renderTargetMetadata.loadOp = renderTargetMetadataJson[LOAD_OP_KEY];

                if (renderTargetMetadataJson.contains(COLOR_CLEAR_VALUE_KEY))
                    renderTargetMetadata.clearValues.color = renderTargetMetadataJson[COLOR_CLEAR_VALUE_KEY];

                if (renderTargetMetadataJson.contains(DEPTH_STENCIL_CLEAR_VALUE_KEY))
                {
                    std::array<float, 2> values = renderTargetMetadataJson[DEPTH_STENCIL_CLEAR_VALUE_KEY];
                    renderTargetMetadata.clearValues.depthStencil.depth = values[0];
                    renderTargetMetadata.clearValues.depthStencil.stencil = (uint32)values[1];
                }
            }

        }
        
        if (!renderPassMetadataJson.contains(PIPELINE_KEY))
        {
            FE_LOG(LogRenderer, ERROR, "No pipeline metadata for render pass {}", renderPassMetadata.renderPassName);
            continue;
        }

        const nlohmann::json& pipelineMetadataJson = renderPassMetadataJson[PIPELINE_KEY];
        
        PipelineName pipelineName = renderPassName;
        if (pipelineMetadataJson.contains(NAME_KEY))
            pipelineName = pipelineMetadataJson[NAME_KEY];

        renderPassMetadata.pipelineName = pipelineName;

        PipelineMetadata& pipelineMetadata = m_pipelineMetadataByName[pipelineName];
        pipelineMetadata.pipelineName = pipelineName;

        if (!pipelineMetadataJson.contains(SHADERS_KEY))
        {
            FE_LOG(LogRenderer, ERROR, "No 'Shaders' key in render pass {}", renderPassName);
            continue;
        }

        const std::vector<JSON>& shaderJsons = pipelineMetadataJson[SHADERS_KEY];
        pipelineMetadata.shadersMetadata.reserve(shaderJsons.size());

        for (const nlohmann::json& shaderMetadataJson : shaderJsons)
        {
            ShaderMetadata& shaderMetadata = pipelineMetadata.shadersMetadata.emplace_back();
            shaderMetadata.type = shaderMetadataJson[TYPE_KEY];
            shaderMetadata.filePath = shaderMetadataJson[PATH_KEY];

            if (shaderMetadataJson.contains(DEFINES_KEY))
                shaderMetadata.defines = shaderMetadataJson[DEFINES_KEY];

            if (shaderMetadataJson.contains(HIT_GROUP_TYPE_KEY))
                shaderMetadata.hitGroupType = shaderMetadataJson[HIT_GROUP_TYPE_KEY];

            if (shaderMetadataJson.contains(ENTRY_POINT_KEY))
                shaderMetadata.entryPoint = shaderMetadataJson[ENTRY_POINT_KEY];

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

const PushConstantsMetadata* RenderGraphMetadata::get_push_constants_metadata(PushConstantsName pushConstantsName) const
{
    auto it = m_pushConstantsMetadataByName.find(pushConstantsName);
    if (it == m_pushConstantsMetadataByName.end())
        return nullptr;
    return &it->second;
}

}