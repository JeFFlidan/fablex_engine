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
constexpr const char* RENDER_PASSES_KEY = "RenderPasses";
constexpr const char* FORMAT_KEY = "Format";
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
constexpr const char* PIPELINES_KEY = "Pipelines";
constexpr const char* SHADERS_KEY = "Shaders";
constexpr const char* SHADER_LIB_KEY = "ShaderLib";
constexpr const char* RAYGEN_KEY = "Raygen";
constexpr const char* MISS_KEY = "Miss";
constexpr const char* CLOSEST_HIT_KEY = "ClosestHit";
constexpr const char* ANY_HIT_KEY = "AnyHitKey";
constexpr const char* INTERSECTION_KEY = "Intersection";
constexpr const char* CALLABLE_KEY = "Callable";
constexpr const char* VERTEX_KEY = "Vertex";
constexpr const char* FRAGMENT_KEY = "Fragment";
constexpr const char* COMPUTE_KEY = "Compute";
constexpr const char* MESH_KEY = "Mesh";
constexpr const char* TASK_KEY = "Task";
constexpr const char* TESSELLATION_EVAL_KEY = "TesselationEval";
constexpr const char* TESSELLATION_CONTROL_KEY = "TesselationControl";
constexpr const char* DEFINES_KEY = "Defines";
constexpr const char* HIT_GROUP_TYPE_KEY = "HitGroupType";
constexpr const char* ENTRY_POINT_KEY = "EntryPoint";
constexpr const char* TYPE_KEY = "Type";
constexpr const char* PATH_KEY = "Path";
constexpr const char* FLAGS_KEY = "Flags";

const std::unordered_map<std::string, rhi::ShaderType> NOT_RT_SHADER_TYPE_BY_KEY_NAME = 
{
    {VERTEX_KEY, rhi::ShaderType::VERTEX},
    {FRAGMENT_KEY, rhi::ShaderType::FRAGMENT},
    {COMPUTE_KEY, rhi::ShaderType::COMPUTE},
    {MESH_KEY, rhi::ShaderType::MESH},
    {TASK_KEY, rhi::ShaderType::TASK},
    {TESSELLATION_EVAL_KEY, rhi::ShaderType::TESSELLATION_EVALUATION},
    {TESSELLATION_CONTROL_KEY, rhi::ShaderType::TESSELLATION_CONTROL}
};

RenderGraphMetadata::RenderGraphMetadata(const RenderContext* renderContext)
{
    FE_CHECK(renderContext);
    m_shaderManager = renderContext->shader_manager();
    FE_CHECK(m_shaderManager);
}

void RenderGraphMetadata::deserialize(const std::string& path)
{
    m_resourceMetadataByName.clear();
    m_renderPassesMetadata.clear();

    FE_LOG(LogRenderer, INFO, "Starting deserializing render graph metadata '{}'", path);

    std::string jsonStr;
    FileSystem::read(FileSystem::get_absolute_path(path), jsonStr);
    JSON json = JSON::parse(jsonStr);

    if (json.contains(RENDER_TEXTURES_KEY))
    {
        const std::vector<JSON>& renderTextureMetadataJsons = json[RENDER_TEXTURES_KEY];
        m_resourceMetadataByName.reserve(m_resourceMetadataByName.size() + renderTextureMetadataJsons.size());

        for (const JSON& textureMetadataJson : renderTextureMetadataJsons)
        {
            ResourceName textureName = textureMetadataJson[NAME_KEY];
            TextureMetadata& textureMetadata = add_metadata<TextureMetadata>(textureName);
            textureMetadata.name = textureName;

            if (textureMetadataJson.contains(FORMAT_KEY))
                textureMetadata.format = textureMetadataJson[FORMAT_KEY];

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

            parse_flags(textureMetadataJson, textureMetadata);
        }
    }

    if (json.contains(PUSH_CONSTANTS_KEY))
    {
        const std::vector<JSON>& pushConstantsMetadataJsons = json[PUSH_CONSTANTS_KEY];
        m_resourceMetadataByName.reserve(m_resourceMetadataByName.size() + pushConstantsMetadataJsons.size());

        for (const JSON& pushConstantsMetadataJson : pushConstantsMetadataJsons)
        {
            PushConstantsName pushConstantsName = pushConstantsMetadataJson[NAME_KEY];
            PushConstantsMetadata& pushConstantsMetadata = add_metadata<PushConstantsMetadata>(pushConstantsName);
            pushConstantsMetadata.name = pushConstantsName;

            if (pushConstantsMetadataJson.contains(RESOURCES_KEY))
            {
                const std::vector<JSON>& resourceMetadataJsons = pushConstantsMetadataJson[RESOURCES_KEY];
                pushConstantsMetadata.resourcesMetadata.reserve(resourceMetadataJsons.size());
                
                for (const JSON& resourceMetadataJson : resourceMetadataJsons)
                {
                    auto& resourceMetadata = pushConstantsMetadata.resourcesMetadata.emplace_back();
                    resourceMetadata.name = resourceMetadataJson[NAME_KEY];

                    parse_flags(resourceMetadataJson, resourceMetadata);
                }
            }
        }
    }

    const std::vector<JSON>& renderPassMetadataJsons = json[RENDER_PASSES_KEY];

    uint64 renderPassAndPipelineCount = m_resourceMetadataByName.size() * 2;    // Because each render pass has its own pipeline
    m_resourceMetadataByName.reserve(m_resourceMetadataByName.size() + renderPassAndPipelineCount);
    m_renderPassesMetadata.reserve(renderPassMetadataJsons.size());

    for (const JSON& renderPassMetadataJson : renderPassMetadataJsons)
    {
        RenderPassName renderPassName = renderPassMetadataJson[NAME_KEY];
        RenderPassMetadata& renderPassMetadata = add_metadata<RenderPassMetadata>(renderPassName);
        m_renderPassesMetadata.push_back(&renderPassMetadata);

        renderPassMetadata.name = renderPassName;

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

            for (const JSON& renderTargetMetadataJson : renderTargetMetadataJsons)
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

        auto fillPipelineMetadata = [&](const JSON& pipelineMetadataJson, PipelineMetadata& pipelineMetadata)
        {
            for (auto& [key, shaderType] : NOT_RT_SHADER_TYPE_BY_KEY_NAME)
            {
                if (pipelineMetadataJson.contains(key))
                {
                    ShaderMetadata& shaderMetadata = pipelineMetadata.shadersMetadata.emplace_back();
                    shaderMetadata.type = shaderType;
                    shaderMetadata.filePath = pipelineMetadataJson[key];
                }
            }

            if (pipelineMetadataJson.contains(SHADERS_KEY))
            {
                const std::vector<JSON>& shaderJsons = pipelineMetadataJson[SHADERS_KEY];
                pipelineMetadata.shadersMetadata.reserve(shaderJsons.size() + pipelineMetadata.shadersMetadata.size());
        
                for (const JSON& shaderMetadataJson : shaderJsons)
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
                }
            }
            else if(pipelineMetadataJson.contains(SHADER_LIB_KEY))
            {
                const JSON& shaderLibMetadata = pipelineMetadataJson[SHADER_LIB_KEY];
                const std::string& shaderLibPath = shaderLibMetadata[PATH_KEY];

                bool isRayTracing = true;

                auto addShaderMetadata = [&](const std::string& entryPoint, rhi::ShaderType shaderType)
                {
                    ShaderMetadata& shaderMetadata = pipelineMetadata.shadersMetadata.emplace_back();
                    shaderMetadata.type = shaderType;
                    shaderMetadata.filePath = shaderLibPath;
                    shaderMetadata.entryPoint = entryPoint;
                };

                for (auto& [key, shaderType] : NOT_RT_SHADER_TYPE_BY_KEY_NAME)
                {
                    if (shaderLibMetadata.contains(key))
                    {
                        isRayTracing = false;
                        addShaderMetadata(shaderLibMetadata[key], shaderType);
                    }
                }

                if (isRayTracing)
                {
                    if (!shaderLibMetadata.contains(RAYGEN_KEY)) FE_CHECK(0);
                    if (!shaderLibMetadata.contains(MISS_KEY)) FE_CHECK(0);
                    if (!shaderLibMetadata.contains(CLOSEST_HIT_KEY)) FE_CHECK(0);

                    const std::vector<std::string>& raygenEntryPoints = shaderLibMetadata[RAYGEN_KEY];
                    const std::vector<std::string>& missEntryPoints = shaderLibMetadata[MISS_KEY];
                    const std::vector<std::string>& closestHitEntryPoints = shaderLibMetadata[CLOSEST_HIT_KEY];

                    std::vector<std::string> anyHitEntryPoints;
                    if (shaderLibMetadata.contains(ANY_HIT_KEY))
                        anyHitEntryPoints = shaderLibMetadata[ANY_HIT_KEY];

                    std::vector<std::string> callableEntryPoints;
                    if (shaderLibMetadata.contains(CALLABLE_KEY))
                        callableEntryPoints = shaderLibMetadata[CALLABLE_KEY];

                    std::vector<std::string> intersectionEntryPoints;
                    if (shaderLibMetadata.contains(INTERSECTION_KEY))
                        intersectionEntryPoints = shaderLibMetadata[INTERSECTION_KEY];

                    pipelineMetadata.shadersMetadata.reserve(
                        raygenEntryPoints.size()
                        + missEntryPoints.size()
                        + closestHitEntryPoints.size()
                        + anyHitEntryPoints.size()
                        + callableEntryPoints.size()
                        + intersectionEntryPoints.size()
                    );

                    for (const std::string& entryPoint : raygenEntryPoints)
                        addShaderMetadata(entryPoint, rhi::ShaderType::RAY_GENERATION);                    

                    for (const std::string& entryPoint : missEntryPoints)
                        addShaderMetadata(entryPoint, rhi::ShaderType::RAY_MISS);

                    uint32 hitGroupIndex = 0;
                    for (const std::string& entryPoint : closestHitEntryPoints)
                    {
                        addShaderMetadata(entryPoint, rhi::ShaderType::RAY_CLOSEST_HIT);
                        if (!anyHitEntryPoints.empty())
                        {
                            const std::string& anyHitEntryPoint = anyHitEntryPoints.at(hitGroupIndex);
                            if (!anyHitEntryPoint.empty())
                                addShaderMetadata(anyHitEntryPoint, rhi::ShaderType::RAY_ANY_HIT);
                        }

                        ++hitGroupIndex;
                    }

                    for (const std::string& entryPoint : intersectionEntryPoints)
                        addShaderMetadata(entryPoint, rhi::ShaderType::RAY_INTERSECTION);

                    for (const std::string& entryPoint : callableEntryPoints)
                        addShaderMetadata(entryPoint, rhi::ShaderType::RAY_CALLABLE);
                }
            }

            if (pipelineMetadata.shadersMetadata.empty())
                FE_LOG(LogRenderer, FATAL, "Pipeline {} does not have any shaders.", pipelineMetadata.name);

            for (const ShaderMetadata& shaderMetadata : pipelineMetadata.shadersMetadata)
                m_shaderManager->request_shader_loading(shaderMetadata);

            for (const RenderTargetMetadata& renderTargetMetadata : renderPassMetadata.renderTargetsMetadata)
            {
                if (rhi::is_depth_stencil_format(renderTargetMetadata.format))
                    pipelineMetadata.depthStencilFormat = renderTargetMetadata.format;
                else
                    pipelineMetadata.colorAttachmentFormats.push_back(renderTargetMetadata.format);
            }
        };
        
        if (renderPassMetadataJson.contains(PIPELINE_KEY))
        {
            const JSON& pipelineMetadataJson = renderPassMetadataJson[PIPELINE_KEY];
            PipelineName pipelineName = renderPassName;
            if (pipelineMetadataJson.contains(NAME_KEY))
                pipelineName = pipelineMetadataJson[NAME_KEY];
            renderPassMetadata.pipelineName = pipelineName;

            PipelineMetadata& pipelineMetadata = add_metadata<PipelineMetadata>(pipelineName);
            pipelineMetadata.name = pipelineName;

            fillPipelineMetadata(pipelineMetadataJson, pipelineMetadata);
        }
        else if (renderPassMetadataJson.contains(PIPELINES_KEY))
        {
            const JSON& pipelinesMetadataJson = renderPassMetadataJson[PIPELINES_KEY];
            std::string generalPipelineName = "";

            for (const JSON& pipelineMetadataJson : pipelinesMetadataJson)
            {
                std::string pipelineName = pipelineMetadataJson[NAME_KEY];
                generalPipelineName += pipelineName + "|";

                PipelineMetadata& pipelineMetadata = add_metadata<PipelineMetadata>(pipelineName);
                pipelineMetadata.name = pipelineName;
                fillPipelineMetadata(pipelineMetadataJson, pipelineMetadata);
            }

            renderPassMetadata.pipelineName = generalPipelineName;
        }
    }

    FE_LOG(LogRenderer, INFO, "Deserializing render graph metadata '{}' completed", path);
}

const TextureMetadata* RenderGraphMetadata::get_texture_metadata(ResourceName textureName) const
{
    auto it = m_resourceMetadataByName.find(textureName);
    if (it == m_resourceMetadataByName.end())
        return nullptr;

    return static_cast<const TextureMetadata*>(it->second.get());
}

const RenderPassMetadata* RenderGraphMetadata::get_render_pass_metadata(RenderPassName renderPassName) const
{
    auto it = m_resourceMetadataByName.find(renderPassName);
    if (it == m_resourceMetadataByName.end())
        return nullptr;

    return static_cast<const RenderPassMetadata*>(it->second.get());
}

const PipelineMetadata* RenderGraphMetadata::get_pipeline_metadata(PipelineName pipelineName) const
{
    auto it = m_resourceMetadataByName.find(pipelineName);
    if (it == m_resourceMetadataByName.end())
        return nullptr;

    return static_cast<const PipelineMetadata*>(it->second.get());
}

const PushConstantsMetadata* RenderGraphMetadata::get_push_constants_metadata(PushConstantsName pushConstantsName) const
{
    auto it = m_resourceMetadataByName.find(pushConstantsName);
    if (it == m_resourceMetadataByName.end())
        return nullptr;
    return static_cast<const PushConstantsMetadata*>(it->second.get());
}

void RenderGraphMetadata::parse_flags(const JSON& metadataJson, ResourceMetadataWithFlags<Name>& metadata)
{
    if (metadataJson.contains(FLAGS_KEY))
    {
        const std::vector<std::string>& flagStrs = metadataJson[FLAGS_KEY];
        for (const std::string& flagStr : flagStrs)
        {
            ResourceMetadataFlag flag;
            to_enum(flagStr, flag);
            metadata.flags |= flag;
        }
    }
}

}