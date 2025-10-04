#pragma once

#include "enums.h"
#include <vector>
#include <string>

namespace fe::rhi
{

struct Shader;

struct ShaderLibrary
{
    rhi::Shader* shader = nullptr;
    rhi::ShaderType type = rhi::ShaderType::RAY_GENERATION;
    std::string entryPoint;
};

struct ShaderHitGroup
{
    enum Type
    {
        GENERAL, // raygen or miss
        TRIANGLES,
        PROCEDURAL
    };

    static constexpr uint32 s_invalidIndex = ~0u;

    Type type = GENERAL;
    ShaderType shaderType = ShaderType::UNDEFINED;
    std::string name;
    uint32 generalShader = s_invalidIndex;
    uint32 closestHitShader = s_invalidIndex;
    uint32 anyHitShader = s_invalidIndex;
    uint32 intersectionShader = s_invalidIndex;
};

struct RayTracingPipelineInfo
{
    std::vector<ShaderLibrary> shaderLibraries;
    std::vector<ShaderHitGroup> shaderHitGroups;
    uint32 maxTraceDepthRecursion = 1;
    uint32 maxAttributeSizeInBytes = 0;
    uint32 maxPayloadSizeInBytes = 0;
};

}