#include "pipeline_manager.h"
#include "shader_manager.h"
#include "device.h"
#include "render_graph/render_pass.h"
#include "render_graph/resource_metadata.h"
#include "render_graph/render_pass_container.h"
#include "core/task_composer.h"
#include "rhi/rhi.h"
#include "rhi/utils.h"
#include "rhi/resources/cmd.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/pipeline.h"
#include "rhi/resources/graphics_pipeline_info.h"
#include "rhi/resources/compute_pipeline_info.h"
#include "rhi/resources/ray_tracing_pipeline_info.h"
#include "core/math.h"
#include "core/utils.h"

namespace fe::renderer
{

PipelineManager::PipelineManager(ShaderManager* shaderManager) : m_shaderManager(shaderManager)
{
    FE_CHECK(m_shaderManager);
    m_taskGroup = TaskComposer::allocate_task_group();
}

PipelineManager::~PipelineManager()
{
    for (auto& [name, identifier] : m_shaderIdentifiersByName)
        rhi::destroy_buffer(identifier.buffer);
}

void PipelineManager::create_graphics_pipeline(const rg::PipelineMetadata& pipelineMetadata)
{
    GraphicsPipelineCreateInfo info;
    configure_pipeline_info(info, pipelineMetadata);

    create_pipeline(pipelineMetadata.name, &info);
}

void PipelineManager::create_graphics_pipeline(
    const rg::PipelineMetadata& pipelineMetadata, 
    const GraphicsPipelineConfigurator& configurator
)
{
    GraphicsPipelineCreateInfo info;
    configure_pipeline_info(info, pipelineMetadata);
    configurator(info);

    create_pipeline(pipelineMetadata.name, &info);
}

void PipelineManager::create_compute_pipeline(const rg::PipelineMetadata& pipelineMetadata)
{
    ComputePipelineCreateInfo info;

    const rg::ShaderMetadata* shaderMetadata = nullptr;
    for (const rg::ShaderMetadata& metadata : pipelineMetadata.shadersMetadata)
    {
        if (metadata.type == ShaderType::COMPUTE)
        {
            shaderMetadata = &metadata;
            break;
        }
    }

    FE_CHECK(shaderMetadata);

    info.shaderStage = m_shaderManager->get_shader(*shaderMetadata);

    create_pipeline(pipelineMetadata.name, &info);
}

void PipelineManager::create_ray_tracing_pipeline(const rg::PipelineMetadata& pipelineMetadata)
{
    RayTracingPipelineCreateInfo info;
    configure_pipeline_info(info, pipelineMetadata);

    create_pipeline(pipelineMetadata.name, &info);
}

void PipelineManager::create_ray_tracing_pipeline(
    const rg::PipelineMetadata& pipelineMetadata, 
    const RayTracingPipelineConfigurator& configurator
)
{
    RayTracingPipelineCreateInfo info;
    configure_pipeline_info(info, pipelineMetadata);
    configurator(info);

    create_pipeline(pipelineMetadata.name, &info);
}

void PipelineManager::create_pipelines(rg::RenderPassContainer* renderPassContainer)
{
    m_shaderManager->wait_shaders_loading();
    rg::RenderPassContainer::RenderPassMap& renderPassMap = renderPassContainer->render_passes();

    for (auto [renderPassName, renderPass] : renderPassMap)
    {
        FE_CHECK(renderPass);

        const rg::RenderPassMetadata& passMetadata = renderPass->metadata();
        if (get_pipeline(passMetadata.pipelineName))
            continue;

        TaskComposer::execute(*m_taskGroup, [renderPass](TaskExecutionInfo execInfo)
        {
            renderPass->create_pipeline();
        });
    }
}

void PipelineManager::wait_pipelines_creation()
{
    TaskComposer::wait(*m_taskGroup);
}

PipelineRef PipelineManager::get_pipeline(PipelineName name) const
{
    auto it = m_pipelineByName.find(name);
    if (it == m_pipelineByName.end())
        return nullptr;
    return it->second;
}

void PipelineManager::bind_pipeline(CommandBufferRef cmd, PipelineName name) const
{
    FE_CHECK(cmd);

    PipelineRef pipeline = get_pipeline(name);
    FE_CHECK(pipeline);

    cmd.bind_pipeline(pipeline);
}

void PipelineManager::push_constants(CommandBufferRef cmd, PipelineName name, void* data) const
{
    FE_CHECK(cmd);
    FE_CHECK(data);

    PipelineRef pipeline = get_pipeline(name);
    if (!pipeline)
        FE_LOG(LogRenderer, FATAL, "Failed to find pipeline {}", name);

    cmd.push_constants(pipeline, data);
}

void PipelineManager::configure_pipeline_info(GraphicsPipelineCreateInfo& outInfo, const rg::PipelineMetadata& pipelineMetadata)
{
    outInfo.assemblyState.topologyType = TopologyType::TRIANGLE;
    
    outInfo.multisampleState.isEnabled = false;
    outInfo.multisampleState.sampleCount = SampleCount::BIT_1;

    outInfo.rasterizationState.cullMode = CullMode::NONE;
    outInfo.rasterizationState.polygonMode = PolygonMode::FILL;
    outInfo.rasterizationState.isBiasEnabled = false;
    outInfo.rasterizationState.frontFace = FrontFace::CLOCKWISE;

    outInfo.colorBlendState.isLogicOpEnabled = false;
    outInfo.colorBlendState.logicOp = LogicOp::COPY;
    ColorBlendAttachmentState& attachState = outInfo.colorBlendState.colorBlendAttachments.emplace_back();
    attachState.isBlendEnabled = false;

    outInfo.depthStencilState.isDepthTestEnabled = false;
    outInfo.depthStencilState.isDepthWriteEnabled = false;
    outInfo.depthStencilState.isStencilTestEnabled = false;
    outInfo.depthStencilState.compareOp = CompareOp::GREATER_OR_EQUAL;

    outInfo.depthFormat = pipelineMetadata.depthStencilFormat;
    outInfo.colorAttachmentFormats = pipelineMetadata.colorAttachmentFormats;

    for (const rg::ShaderMetadata& shaderMetadata : pipelineMetadata.shadersMetadata)
    {
        outInfo.shaderStages.push_back(m_shaderManager->get_shader(shaderMetadata));
    }
}

void PipelineManager::configure_pipeline_info(RayTracingPipelineCreateInfo& outInfo, const rg::PipelineMetadata& pipelineMetadata)
{
    auto getHitGroupName = [](std::string entryPoint, ShaderType shaderType)
    {
        std::string wordToRemove;

        switch (shaderType)
        {
        case ShaderType::RAY_ANY_HIT:
            wordToRemove = "any";
            break;
        case ShaderType::RAY_CLOSEST_HIT:
            wordToRemove = "closest";
            break;
        default:
            return std::string();
        }

        size_t pos = Utils::to_lower(entryPoint).find(wordToRemove);
        FE_CHECK(pos != std::string::npos);
        return entryPoint.erase(pos, wordToRemove.length());
    };

    auto fillGeometryHitGroup = [getHitGroupName](
        const rg::ShaderMetadata& shaderMetadata,
        std::vector<ShaderHitGroup>& hitGroups,
        const std::vector<ShaderLibrary>& shaderLibs
    )
    {
        if (shaderMetadata.type != ShaderType::RAY_CLOSEST_HIT && shaderMetadata.type != ShaderType::RAY_ANY_HIT)
            return;

        ShaderHitGroup* hitGroup = nullptr;
        
        switch (hitGroups.back().type)
        {
        case ShaderHitGroup::TRIANGLES:
        case ShaderHitGroup::PROCEDURAL:
        {
            if (hitGroups.back().type == shaderMetadata.hitGroupType)
                hitGroup = &hitGroups.back();
            else
                hitGroup = &hitGroups.emplace_back();

            break;
        }
        case ShaderHitGroup::GENERAL:
        {
            hitGroup = &hitGroups.emplace_back();
            break;
        }
        }

        uint32* hitShaderIndex = nullptr;

        switch (shaderMetadata.type)
        {
        case ShaderType::RAY_CLOSEST_HIT:
        {
            if (hitGroup->closestHitShader != ShaderHitGroup::s_invalidIndex)
                hitGroup = &hitGroups.emplace_back();
            hitShaderIndex = &hitGroup->closestHitShader;
            break;
        }
        case ShaderType::RAY_ANY_HIT:
        {
            if (hitGroup->anyHitShader != ShaderHitGroup::s_invalidIndex)
                hitGroup = &hitGroups.emplace_back();
            hitShaderIndex = &hitGroup->anyHitShader;
            break;
        }
        default:
            return;
        }

        hitGroup->type = shaderMetadata.hitGroupType;
        hitGroup->shaderType = shaderMetadata.type;

        if (hitGroup->name.empty())
            hitGroup->name = getHitGroupName(shaderMetadata.entryPoint, shaderMetadata.type);

        *hitShaderIndex = shaderLibs.size() - 1;
    };

    const std::vector<rg::ShaderMetadata>& shadersMetadata = pipelineMetadata.shadersMetadata;
    for (uint32 i = 0; i != shadersMetadata.size(); ++i)
    {
        const rg::ShaderMetadata& shaderMetadata = shadersMetadata[i];
        ShaderRef shader = m_shaderManager->get_shader(shaderMetadata);
        FE_CHECK(shader);

        ShaderLibrary& shaderLibrary = outInfo.shaderLibraries.emplace_back();
        shaderLibrary.shader = shader;
        shaderLibrary.type = shaderMetadata.type;
        shaderLibrary.entryPoint = shaderMetadata.entryPoint;

        switch (shaderMetadata.type)
        {
        case ShaderType::RAY_GENERATION:
        case ShaderType::RAY_MISS:
        {
            ShaderHitGroup& hitGroup = outInfo.shaderHitGroups.emplace_back();
            hitGroup.type = ShaderHitGroup::GENERAL;
            hitGroup.shaderType = shaderMetadata.type;
            hitGroup.generalShader = outInfo.shaderLibraries.size() - 1;
            hitGroup.name = shaderMetadata.entryPoint;
            break;
        }
        case ShaderType::RAY_CLOSEST_HIT:
        case ShaderType::RAY_ANY_HIT:
            fillGeometryHitGroup(shaderMetadata, outInfo.shaderHitGroups, outInfo.shaderLibraries);
            break;
        default:
            continue;
        }
    }

    outInfo.maxTraceDepthRecursion = 1;
    outInfo.maxPayloadSizeInBytes = 128;
    outInfo.maxAttributeSizeInBytes = sizeof(Float2);
}

void PipelineManager::create_pipeline(PipelineName pipelineName, PipelineInfoVariant infoVariant)
{
    PipelineHandle pipeline;

    if (GraphicsPipelineCreateInfo** info = std::get_if<GraphicsPipelineCreateInfo*>(&infoVariant))
    {
        pipeline.init(**info);
    }
    else if (ComputePipelineCreateInfo** info = std::get_if<ComputePipelineCreateInfo*>(&infoVariant))
    {
        pipeline.init(**info);
    }
    else if (RayTracingPipelineCreateInfo** info = std::get_if<RayTracingPipelineCreateInfo*>(&infoVariant))
    {
        pipeline.init(**info);

        {
            std::scoped_lock<std::mutex> locker(m_shaderIdentifiersMapMutex);
            m_shaderIdentifiersByName[pipelineName];
        }

        const uint64 identifierSize = Device::shader_identifier_size();
        const uint64 identifierAlignment = Device::shader_identifier_alignment();

        RayTracingPipelineCreateInfo& infoRef = **info;

        ShaderIdentifierBuffer& identifierBuffer = m_shaderIdentifiersByName[pipelineName];
        if (identifierBuffer.buffer != nullptr)
            FE_LOG(LogRenderer, FATAL, "Pipeline name {} is not unique.", pipelineName);

        identifierBuffer.stride = identifierSize;

        BufferCreateInfo bufferInfo;
        bufferInfo.bufferUsage = ResourceUsage::STORAGE_BUFFER;
        bufferInfo.memoryUsage = MemoryUsage::CPU_TO_GPU;
        bufferInfo.flags = ResourceFlags::RAY_TRACING;

        uint32 raygenIdentifierCount = 0;
        uint32 missShaderIdentifierCount = 0;
        uint32 hitShaderIdentifierCount = 0;
        uint32 callableShaderIdentifierCount = 0;

        for (const ShaderHitGroup& hitGroup : infoRef.shaderHitGroups)
        {
            switch (hitGroup.shaderType)
            {
            case ShaderType::RAY_GENERATION:
                ++raygenIdentifierCount; 
                break;
            case ShaderType::RAY_MISS: 
                ++missShaderIdentifierCount; 
                break;
            case ShaderType::RAY_CALLABLE: 
                ++callableShaderIdentifierCount; 
                break;
            case ShaderType::RAY_ANY_HIT:
            case ShaderType::RAY_CLOSEST_HIT:
            case ShaderType::RAY_INTERSECTION:
                ++hitShaderIdentifierCount;
                break;
            default:
                FE_CHECK(0);
            }
        }

        uint64 globalOffset = 0;

        identifierBuffer.raygenIdentifier.size = raygenIdentifierCount * identifierSize;
        identifierBuffer.raygenIdentifier.offset = 0;
        globalOffset += rhi::align_to(identifierBuffer.raygenIdentifier.size, identifierAlignment);

        identifierBuffer.missIdentifier.size = missShaderIdentifierCount * identifierSize;
        identifierBuffer.missIdentifier.offset = globalOffset;
        globalOffset += rhi::align_to(identifierBuffer.missIdentifier.size, identifierAlignment);

        identifierBuffer.hitGroupIdentifier.size = hitShaderIdentifierCount * identifierSize;
        identifierBuffer.hitGroupIdentifier.offset = globalOffset;
        globalOffset += rhi::align_to(identifierBuffer.hitGroupIdentifier.size, identifierAlignment);

        identifierBuffer.callableIdentifier.size = callableShaderIdentifierCount * identifierSize;
        identifierBuffer.callableIdentifier.offset = globalOffset;
        globalOffset += rhi::align_to(identifierBuffer.callableIdentifier.size, identifierAlignment);

        bufferInfo.size = globalOffset;
        rhi::create_buffer(&identifierBuffer.buffer, &bufferInfo);

        uint32 groupIndex = 0;

        uint64 raygenLocalOffset = 0;
        uint64 missLocalOffset = identifierBuffer.missIdentifier.offset;
        uint64 hitLocalOffset = identifierBuffer.hitGroupIdentifier.offset;
        uint64 callableLocalOffset = identifierBuffer.callableIdentifier.offset;

        uint8* mappedData = static_cast<uint8*>(identifierBuffer.buffer->mappedData);

        for (const ShaderHitGroup& shaderHitGroup : infoRef.shaderHitGroups)
        {
            switch (shaderHitGroup.shaderType)
            {
            case ShaderType::RAY_GENERATION:
            {
                rhi::write_shader_identifier(pipeline, groupIndex, mappedData + raygenLocalOffset);
                raygenLocalOffset += identifierSize;
                break;
            }
            case ShaderType::RAY_MISS:
            {
                rhi::write_shader_identifier(pipeline, groupIndex, mappedData + missLocalOffset);
                missLocalOffset += identifierSize;
                break;
            }
            case ShaderType::RAY_INTERSECTION:
            case ShaderType::RAY_CLOSEST_HIT:
            case ShaderType::RAY_ANY_HIT:
            {
                rhi::write_shader_identifier(pipeline, groupIndex, mappedData + hitLocalOffset);
                hitLocalOffset += identifierSize;
                break;
            }
            case ShaderType::RAY_CALLABLE:
            {
                rhi::write_shader_identifier(pipeline, groupIndex, mappedData + callableLocalOffset);
                callableLocalOffset += identifierSize;
                break;
            }
            default: FE_CHECK(0);
            }
            
            ++groupIndex;
        }
    }

    pipeline.set_name(pipelineName.to_string());

    std::scoped_lock<std::mutex> locker(m_pipelineMapMutex);
    m_pipelineByName[pipelineName] = std::move(pipeline);
}

void PipelineManager::fill_dispatch_rays_info(PipelineName name, DispatchRaysInfo& outInfo) const
{
    outInfo.shaderIdentifierBuffer = m_shaderIdentifiersByName.at(name);
}

}