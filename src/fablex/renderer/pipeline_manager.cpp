#include "pipeline_manager.h"
#include "shader_manager.h"
#include "rhi/rhi.h"
#include "core/math.h"

namespace fe::renderer
{

void PipelineManager::create_graphics_pipeline(const PipelineMetadata& pipelineMetadata)
{
    rhi::GraphicsPipelineInfo info;
    configure_pipeline_info(info, pipelineMetadata);

    create_pipeline(pipelineMetadata.pipelineName, &info);
}

void PipelineManager::create_graphics_pipeline(
    const PipelineMetadata& pipelineMetadata, 
    const GraphicsPipelineConfigurator& configurator
)
{
    rhi::GraphicsPipelineInfo info;
    configure_pipeline_info(info, pipelineMetadata);
    configurator(info);

    create_pipeline(pipelineMetadata.pipelineName, &info);
}

void PipelineManager::create_compute_pipeline(const PipelineMetadata& pipelineMetadata)
{
    rhi::ComputePipelineInfo info;

    const ShaderMetadata* shaderMetadata = nullptr;
    for (const ShaderMetadata& metadata : pipelineMetadata.shadersMetadata)
    {
        if (metadata.type == rhi::ShaderType::COMPUTE)
        {
            shaderMetadata = &metadata;
            break;
        }
    }

    FE_CHECK(shaderMetadata);

    info.shaderStage = ShaderManager::get_shader(*shaderMetadata);

    create_pipeline(pipelineMetadata.pipelineName, &info);
}

void PipelineManager::create_ray_tracing_pipeline(const PipelineMetadata& pipelineMetadata)
{
    rhi::RayTracingPipelineInfo info;
    configure_pipeline_info(info, pipelineMetadata);

    create_pipeline(pipelineMetadata.pipelineName, &info);
}

void PipelineManager::create_ray_tracing_pipeline(
    const PipelineMetadata& pipelineMetadata, 
    const RayTracingPipelineConfigurator& configurator
)
{
    rhi::RayTracingPipelineInfo info;
    configure_pipeline_info(info, pipelineMetadata);
    configurator(info);

    create_pipeline(pipelineMetadata.pipelineName, &info);
}

rhi::Pipeline* PipelineManager::get_pipeline(PipelineName name) const
{
    auto it = m_pipelineByName.find(name);
    if (it == m_pipelineByName.end())
        return nullptr;
    return it->second;
}

void PipelineManager::bind_pipeline(rhi::CommandBuffer* cmd, PipelineName name) const
{
    rhi::Pipeline* pipeline = get_pipeline(name);
    FE_CHECK(pipeline);

    rhi::bind_pipeline(cmd, pipeline);
}

void PipelineManager::configure_pipeline_info(rhi::GraphicsPipelineInfo& outInfo, const PipelineMetadata& pipelineMetadata)
{
    outInfo.assemblyState.topologyType = rhi::TopologyType::TRIANGLE;
    
    outInfo.multisampleState.isEnabled = false;
    outInfo.multisampleState.sampleCount = rhi::SampleCount::BIT_1;

    outInfo.rasterizationState.cullMode = rhi::CullMode::BACK;
    outInfo.rasterizationState.polygonMode = rhi::PolygonMode::FILL;
    outInfo.rasterizationState.isBiasEnabled = false;
    outInfo.rasterizationState.frontFace = rhi::FrontFace::CLOCKWISE;

    outInfo.colorBlendState.isLogicOpEnabled = false;
    outInfo.colorBlendState.logicOp = rhi::LogicOp::COPY;
    rhi::ColorBlendAttachmentState& attachState = outInfo.colorBlendState.colorBlendAttachments.emplace_back();
    attachState.isBlendEnabled = false;

    outInfo.depthStencilState.isDepthTestEnabled = false;
    outInfo.depthStencilState.isDepthWriteEnabled = false;
    outInfo.depthStencilState.isStencilTestEnabled = false;
    outInfo.depthStencilState.compareOp = rhi::CompareOp::GREATER_OR_EQUAL;

    outInfo.depthFormat = pipelineMetadata.depthStencilFormat;
    outInfo.colorAttachmentFormats = pipelineMetadata.colorAttachmentFormats;

    for (const ShaderMetadata& shaderMetadata : pipelineMetadata.shadersMetadata)
    {
        outInfo.shaderStages.push_back(ShaderManager::get_shader(shaderMetadata));
    }
}

void PipelineManager::configure_pipeline_info(rhi::RayTracingPipelineInfo& outInfo, const PipelineMetadata& pipelineMetadata)
{
    auto getHitGroupName = [](std::string entryPoint, rhi::ShaderType shaderType)
    {
        std::string wordToRemove;

        switch (shaderType)
        {
        case rhi::ShaderType::RAY_ANY_HIT:
            wordToRemove = "Any";
            break;
        case rhi::ShaderType::RAY_CLOSEST_HIT:
            wordToRemove = "Closest";
            break;
        default:
            return std::string();
        }

        size_t pos = entryPoint.find(wordToRemove);
        FE_CHECK(pos != std::string::npos);
        return entryPoint.erase(pos, wordToRemove.length());
    };

    auto fillGeometryHitGroup = [getHitGroupName](
        const ShaderMetadata& shaderMetadata,
        std::vector<rhi::ShaderHitGroup>& hitGroups,
        const std::vector<rhi::ShaderLibrary>& shaderLibs
    )
    {
        if (shaderMetadata.type != rhi::ShaderType::RAY_CLOSEST_HIT && shaderMetadata.type != rhi::ShaderType::RAY_ANY_HIT)
            return;

        rhi::ShaderHitGroup* hitGroup = nullptr;
        
        switch (hitGroups.back().type)
        {
        case rhi::ShaderHitGroup::TRIANGLES:
        case rhi::ShaderHitGroup::PROCEDURAL:
            if (hitGroups.back().type == shaderMetadata.hitGroupType)
                hitGroup = &hitGroups.back();
            else
                hitGroup = &hitGroups.emplace_back();
        case rhi::ShaderHitGroup::GENERAL:
            hitGroup = &hitGroups.emplace_back();
        }

        uint32* hitShaderIndex = nullptr;

        switch (shaderMetadata.type)
        {
        case rhi::ShaderType::RAY_CLOSEST_HIT:
        {
            if (hitGroup->closestHitShader != rhi::ShaderHitGroup::s_invalidIndex)
                hitGroup = &hitGroups.emplace_back();
            hitShaderIndex = &hitGroup->closestHitShader;
            break;
        }
        case rhi::ShaderType::RAY_ANY_HIT:
        {
            if (hitGroup->anyHitShader != rhi::ShaderHitGroup::s_invalidIndex)
                hitGroup = &hitGroups.emplace_back();
            hitShaderIndex = &hitGroup->anyHitShader;
            break;
        }
        default:
            return;
        }

        hitGroup->type = shaderMetadata.hitGroupType;

        if (hitGroup->name.empty())
            hitGroup->name = getHitGroupName(shaderMetadata.entryPoint, shaderMetadata.type);

        *hitShaderIndex = shaderLibs.size() - 1;
    };

    const std::vector<ShaderMetadata>& shadersMetadata = pipelineMetadata.shadersMetadata;
    for (uint32 i = 0; i != shadersMetadata.size(); ++i)
    {
        const ShaderMetadata& shaderMetadata = shadersMetadata[i];
        rhi::Shader* shader = ShaderManager::get_shader(shaderMetadata);
        FE_CHECK(shader);

        rhi::ShaderLibrary& shaderLibrary = outInfo.shaderLibraries.emplace_back();
        shaderLibrary.shader = shader;
        shaderLibrary.type = shaderMetadata.type;
        shaderLibrary.entryPoint = shaderMetadata.entryPoint;

        switch (shaderMetadata.type)
        {
        case rhi::ShaderType::RAY_GENERATION:
        case rhi::ShaderType::RAY_MISS:
        {
            rhi::ShaderHitGroup& hitGroup = outInfo.shaderHitGroups.emplace_back();
            hitGroup.type = rhi::ShaderHitGroup::GENERAL;
            hitGroup.generalShader = outInfo.shaderLibraries.size() - 1;
            hitGroup.name = shaderMetadata.entryPoint;
            break;
        }
        case rhi::ShaderType::RAY_CLOSEST_HIT:
        case rhi::ShaderType::RAY_ANY_HIT:
            fillGeometryHitGroup(shaderMetadata, outInfo.shaderHitGroups, outInfo.shaderLibraries);
            break;
        default:
            continue;
        }
    }

    outInfo.maxTraceDepthRecursion = 1;
    outInfo.maxPayloadSizeInBytes = sizeof(math::Float4);
    outInfo.maxAttributeSizeInBytes = sizeof(math::Float2);
}

void PipelineManager::create_pipeline(PipelineName pipelineName, PipelineInfoVariant infoVariant)
{
    rhi::Pipeline* pipeline = nullptr;

    if (rhi::GraphicsPipelineInfo** info = std::get_if<rhi::GraphicsPipelineInfo*>(&infoVariant))
        rhi::create_graphics_pipeline(&pipeline, *info);
    else if (rhi::ComputePipelineInfo** info = std::get_if<rhi::ComputePipelineInfo*>(&infoVariant))
        rhi::create_compute_pipeline(&pipeline, *info);
    else if (rhi::RayTracingPipelineInfo** info = std::get_if<rhi::RayTracingPipelineInfo*>(&infoVariant))
        rhi::create_ray_tracing_pipeline(&pipeline, *info);

    std::scoped_lock<std::mutex> locker(m_mutex);
    m_pipelineByName[pipelineName] = pipeline;
}

}