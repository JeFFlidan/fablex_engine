#include "shader_identifiers.h"
#include "device.h"
#include "rhi/utils.h"
#include "rhi/resources/ray_tracing_pipeline_info.h"

namespace fe::renderer
{

void ShaderIdentifiers::init(PipelineRef pipeline, const RayTracingPipelineCreateInfo& createInfo)
{
    const uint64 identifierSize = Device::shader_identifier_size();
    const uint64 identifierAlignment = Device::shader_identifier_alignment();

    BufferCreateInfo bufferInfo;
    bufferInfo.bufferUsage = ResourceUsage::STORAGE_BUFFER;
    bufferInfo.memoryUsage = MemoryUsage::CPU_TO_GPU;
    bufferInfo.flags = ResourceFlags::RAY_TRACING;

    uint32 raygenIdentifierCount = 0;
    uint32 missShaderIdentifierCount = 0;
    uint32 hitShaderIdentifierCount = 0;
    uint32 callableShaderIdentifierCount = 0;

    for (const ShaderHitGroup& hitGroup : createInfo.shaderHitGroups)
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

    raygenIdentifier.size = raygenIdentifierCount * identifierSize;
    raygenIdentifier.offset = 0;
    globalOffset += rhi::align_to(raygenIdentifier.size, identifierAlignment);

    missIdentifier.size = missShaderIdentifierCount * identifierSize;
    missIdentifier.offset = globalOffset;
    globalOffset += rhi::align_to(missIdentifier.size, identifierAlignment);

    hitGroupIdentifier.size = hitShaderIdentifierCount * identifierSize;
    hitGroupIdentifier.offset = globalOffset;
    globalOffset += rhi::align_to(hitGroupIdentifier.size, identifierAlignment);

    callableIdentifier.size = callableShaderIdentifierCount * identifierSize;
    callableIdentifier.offset = globalOffset;
    globalOffset += rhi::align_to(callableIdentifier.size, identifierAlignment);

    bufferInfo.size = globalOffset;

    m_buffer.init(bufferInfo);

    uint32 groupIndex = 0;

    uint64 raygenLocalOffset = 0;
    uint64 missLocalOffset = missIdentifier.offset;
    uint64 hitLocalOffset = hitGroupIdentifier.offset;
    uint64 callableLocalOffset = callableIdentifier.offset;

    uint8* mappedData = m_buffer.mapped_data();

    for (const ShaderHitGroup& shaderHitGroup : createInfo.shaderHitGroups)
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

void ShaderIdentifiers::fill_dispatch_rays_info(DispatchRaysInfo& outRaysInfo) const
{
    ShaderIdentifierBuffer& identifierBuffer = outRaysInfo.shaderIdentifierBuffer;
    identifierBuffer.stride = Device::shader_identifier_size();
    identifierBuffer.raygenIdentifier = raygenIdentifier;
    identifierBuffer.missIdentifier = missIdentifier;
    identifierBuffer.hitGroupIdentifier = hitGroupIdentifier;
    identifierBuffer.callableIdentifier = callableIdentifier;
    identifierBuffer.buffer = m_buffer;
}

}