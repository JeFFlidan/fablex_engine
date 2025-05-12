#include "pipeline_manager.h"
#include "shader_manager.h"
#include "render_pass_container.h"
#include "render_pass.h"
#include "core/task_composer.h"
#include "rhi/rhi.h"
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
    for (auto [name, pipeline] : m_pipelineByName)
        rhi::destroy_pipeline(pipeline);

    for (auto& [name, identifiers] : m_shaderIdentifiersByName)
        for (const rhi::ShaderIdentifierBuffer& identifier : identifiers)
            rhi::destroy_buffer(identifier.buffer);
}

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

    info.shaderStage = m_shaderManager->get_shader(*shaderMetadata);

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

void PipelineManager::create_pipelines(RenderPassContainer* renderPassContainer)
{
    m_shaderManager->wait_shaders_loading();
    RenderPassContainer::RenderPassMap& renderPassMap = renderPassContainer->get_render_passes();

    for (auto [renderPassName, renderPass] : renderPassMap)
    {
        FE_CHECK(renderPass);

        const RenderPassMetadata& passMetadata = renderPass->get_metadata();
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

rhi::Pipeline* PipelineManager::get_pipeline(PipelineName name) const
{
    auto it = m_pipelineByName.find(name);
    if (it == m_pipelineByName.end())
        return nullptr;
    return it->second;
}

void PipelineManager::bind_pipeline(rhi::CommandBuffer* cmd, PipelineName name) const
{
    FE_CHECK(cmd);

    rhi::Pipeline* pipeline = get_pipeline(name);
    FE_CHECK(pipeline);

    rhi::bind_pipeline(cmd, pipeline);
}

void PipelineManager::push_constants(rhi::CommandBuffer* cmd, PipelineName name, void* data) const
{
    FE_CHECK(cmd);
    FE_CHECK(data);

    rhi::Pipeline* pipeline = get_pipeline(name);
    FE_CHECK(pipeline);

    rhi::push_constants(cmd, pipeline, data);
}

void PipelineManager::configure_pipeline_info(rhi::GraphicsPipelineInfo& outInfo, const PipelineMetadata& pipelineMetadata)
{
    outInfo.assemblyState.topologyType = rhi::TopologyType::TRIANGLE;
    
    outInfo.multisampleState.isEnabled = false;
    outInfo.multisampleState.sampleCount = rhi::SampleCount::BIT_1;

    outInfo.rasterizationState.cullMode = rhi::CullMode::NONE;
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
        outInfo.shaderStages.push_back(m_shaderManager->get_shader(shaderMetadata));
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
            wordToRemove = "any";
            break;
        case rhi::ShaderType::RAY_CLOSEST_HIT:
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
        {
            if (hitGroups.back().type == shaderMetadata.hitGroupType)
                hitGroup = &hitGroups.back();
            else
                hitGroup = &hitGroups.emplace_back();

            break;
        }
        case rhi::ShaderHitGroup::GENERAL:
        {
            hitGroup = &hitGroups.emplace_back();
            break;
        }
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
        hitGroup->shaderType = shaderMetadata.type;

        if (hitGroup->name.empty())
            hitGroup->name = getHitGroupName(shaderMetadata.entryPoint, shaderMetadata.type);

        *hitShaderIndex = shaderLibs.size() - 1;
    };

    const std::vector<ShaderMetadata>& shadersMetadata = pipelineMetadata.shadersMetadata;
    for (uint32 i = 0; i != shadersMetadata.size(); ++i)
    {
        const ShaderMetadata& shaderMetadata = shadersMetadata[i];
        rhi::Shader* shader = m_shaderManager->get_shader(shaderMetadata);
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
            hitGroup.shaderType = shaderMetadata.type;
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
    outInfo.maxPayloadSizeInBytes = sizeof(Float4) * 4;
    outInfo.maxAttributeSizeInBytes = sizeof(Float2);
}

void PipelineManager::create_pipeline(PipelineName pipelineName, PipelineInfoVariant infoVariant)
{
    rhi::Pipeline* pipeline = nullptr;

    if (rhi::GraphicsPipelineInfo** info = std::get_if<rhi::GraphicsPipelineInfo*>(&infoVariant))
    {
        rhi::create_graphics_pipeline(&pipeline, *info);
    }
    else if (rhi::ComputePipelineInfo** info = std::get_if<rhi::ComputePipelineInfo*>(&infoVariant))
    {
        rhi::create_compute_pipeline(&pipeline, *info);
    }
    else if (rhi::RayTracingPipelineInfo** info = std::get_if<rhi::RayTracingPipelineInfo*>(&infoVariant))
    {
        rhi::create_ray_tracing_pipeline(&pipeline, *info);

        {
            std::scoped_lock<std::mutex> locker(m_shaderIdentifiersMapMutex);
            m_shaderIdentifiersByName[pipelineName];
        }

        const uint64 identifierSize = rhi::get_shader_identifier_size();
        rhi::RayTracingPipelineInfo& infoRef = **info;

        uint32 raygenIdentifierCount = 0;
        uint32 missShaderIdentifierCount = 0;
        uint32 hitShaderIdentifierCount = 0;
        uint32 callableShaderIdentifierCount = 0;

        for (const rhi::ShaderHitGroup& hitGroup : infoRef.shaderHitGroups)
        {
            switch (hitGroup.shaderType)
            {
            case rhi::ShaderType::RAY_GENERATION:
                ++raygenIdentifierCount; 
                break;
            case rhi::ShaderType::RAY_MISS: 
                ++missShaderIdentifierCount; 
                break;
            case rhi::ShaderType::RAY_CALLABLE: 
                ++callableShaderIdentifierCount; 
                break;
            case rhi::ShaderType::RAY_ANY_HIT:
            case rhi::ShaderType::RAY_CLOSEST_HIT:
            case rhi::ShaderType::RAY_INTERSECTION:
                ++hitShaderIdentifierCount;
                break;
            default:
                FE_CHECK(0);
            }
        }

        auto createIdentifierBuffer = [&](rhi::ShaderIdentifierBuffer::Type type)
        {
            uint32 identifierCount = 0;
            switch (type)
            {
            case rhi::ShaderIdentifierBuffer::RAY_GENERATION:
                identifierCount = raygenIdentifierCount;
                break;
            case rhi::ShaderIdentifierBuffer::RAY_MISS:
                identifierCount = missShaderIdentifierCount;
                break;
            case rhi::ShaderIdentifierBuffer::RAY_HIT:
                identifierCount = hitShaderIdentifierCount;
                break;
            case rhi::ShaderIdentifierBuffer::RAY_CALLABLE:
                identifierCount = callableShaderIdentifierCount;
                break;
            default:
                FE_CHECK(0);
            }

            rhi::BufferInfo bufferInfo;
            bufferInfo.bufferUsage = rhi::ResourceUsage::STORAGE_BUFFER;
            bufferInfo.memoryUsage = rhi::MemoryUsage::CPU_TO_GPU;
            bufferInfo.flags = rhi::ResourceFlags::RAY_TRACING;
            bufferInfo.size = identifierSize * identifierCount;

            rhi::Buffer* buffer;
            rhi::create_buffer(&buffer, &bufferInfo);
            return buffer;
        };

        auto getIdentifierType = [&](const rhi::ShaderHitGroup& hitGroup)
        {
            switch (hitGroup.shaderType)
            {
            case rhi::ShaderType::RAY_GENERATION:
                return rhi::ShaderIdentifierBuffer::RAY_GENERATION;
            case rhi::ShaderType::RAY_MISS:
                return rhi::ShaderIdentifierBuffer::RAY_MISS;
            case rhi::ShaderType::RAY_ANY_HIT:
            case rhi::ShaderType::RAY_CLOSEST_HIT:
            case rhi::ShaderType::RAY_INTERSECTION:
                return rhi::ShaderIdentifierBuffer::RAY_HIT;
            case rhi::ShaderType::RAY_CALLABLE:
                return rhi::ShaderIdentifierBuffer::RAY_CALLABLE;
            default:
                FE_CHECK(0);
                return rhi::ShaderIdentifierBuffer::UNDEFINED;   // To prevent compilation warning
            }
        };

        uint32 groupIndex = 0;

        std::unordered_map<rhi::ShaderIdentifierBuffer::Type, uint32> identifierIdxByType;

        for (const rhi::ShaderHitGroup& shaderHitGroup : infoRef.shaderHitGroups)
        {
            auto identifierType = getIdentifierType(shaderHitGroup);
            ShaderIdentifierArray& shaderIdentifierArray = m_shaderIdentifiersByName[pipelineName];
            
            auto it = identifierIdxByType.find(identifierType);
            if (it == identifierIdxByType.end())
            {
                rhi::ShaderIdentifierBuffer& identifier = shaderIdentifierArray.emplace_back();
                identifier.name = shaderHitGroup.name;
                identifier.type = identifierType;
                identifier.buffer = createIdentifierBuffer(identifierType);
                identifier.size = identifier.buffer->size;
                identifier.stride = identifierSize;
                identifier.offset = 0;

                identifierIdxByType[identifierType] = shaderIdentifierArray.size() - 1;
            }

            rhi::ShaderIdentifierBuffer& identifier = shaderIdentifierArray.at(identifierIdxByType[identifierType]);
            uint8* mappedData = static_cast<uint8*>(identifier.buffer->mappedData) + identifier.offset;
            
            rhi::write_shader_identifier(pipeline, groupIndex, mappedData);
            identifier.offset += identifierSize;
            
            ++groupIndex;
        }

        for (rhi::ShaderIdentifierBuffer& identifier : m_shaderIdentifiersByName[pipelineName])
            identifier.offset = 0;
    }

    std::scoped_lock<std::mutex> locker(m_pipelineMapMutex);
    m_pipelineByName[pipelineName] = pipeline;

    rhi::set_name(pipeline, pipelineName.to_string());
}

void PipelineManager::fill_dispatch_rays_info(PipelineName name, rhi::DispatchRaysInfo& outInfo) const
{
    const ShaderIdentifierArray& shaderIdentifierArray = m_shaderIdentifiersByName.at(name);

    for (const rhi::ShaderIdentifierBuffer& identifier : shaderIdentifierArray)
    {
        switch (identifier.type)
        {
        case rhi::ShaderIdentifierBuffer::RAY_GENERATION:
            outInfo.rayGeneration = identifier;
            break;
        case rhi::ShaderIdentifierBuffer::RAY_MISS:
            outInfo.miss = identifier;
            break;
        case rhi::ShaderIdentifierBuffer::RAY_HIT:
            outInfo.hitGroup = identifier;
            break;
        case rhi::ShaderIdentifierBuffer::RAY_CALLABLE:
            outInfo.callable = identifier;
            break;
        default:
            FE_CHECK(0);
        }
    }
}

}