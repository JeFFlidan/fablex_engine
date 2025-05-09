#pragma once

#include "core/name.h"
#include "core/logger.h"
#include "core/window.h"
#include "rhi/resources.h"
#include "core/utils.h"

#include <functional>

namespace fe::renderer
{

using RenderPassName = Name;
using ResourceName = Name;
using PipelineName = Name;
using FieldName = Name;
using PushConstantsName = Name;

FE_DEFINE_LOG_CATEGORY(LogRenderer);

struct TextureMetadata
{
    ResourceName textureName;
    uint32 layerCount = 1;
    rhi::SampleCount sampleCount = rhi::SampleCount::UNDEFINED;
    rhi::Format format = rhi::Format::UNDEFINED;
    bool useMips = false;
    bool isTransferDst = false;
    bool crossFrameRead = false;
};

struct RenderTargetMetadata
{
    ResourceName textureName;
    rhi::Format format = rhi::Format::UNDEFINED;
    rhi::StoreOp storeOp = rhi::StoreOp::STORE;
    rhi::LoadOp loadOp = rhi::LoadOp::CLEAR;
    rhi::ClearValues clearValues;
};

enum class RenderPassType
{
    GRAPHICS,
    ASYNC_COMPUTE
};

struct RenderPassMetadata
{
    RenderPassName renderPassName;
    RenderPassType type = RenderPassType::GRAPHICS;
    PipelineName pipelineName;
    PushConstantsName pushConstantsName;
    std::vector<ResourceName> inputTextureNames;
    std::vector<RenderTargetMetadata> renderTargetsMetadata;
    std::vector<ResourceName> outputStorageTextureNames;
};

struct ShaderMetadata
{
    std::string filePath;
    rhi::ShaderType type;
    rhi::ShaderHitGroup::Type hitGroupType = rhi::ShaderHitGroup::TRIANGLES;    // Only for hit shaders
    std::string entryPoint;
    std::vector<std::string> defines;

    bool operator==(const ShaderMetadata& other) const
    {
        return filePath == other.filePath
            && type == other.type
            && hitGroupType == other.hitGroupType
            && entryPoint == other.entryPoint
            && defines == other.defines;
    }
};

struct PipelineMetadata
{
    PipelineName pipelineName;
    std::vector<ShaderMetadata> shadersMetadata;
    std::vector<rhi::Format> colorAttachmentFormats;
    rhi::Format depthStencilFormat = rhi::Format::UNDEFINED;
};

enum class PushConstantType
{
    FLOAT,
    INT32,
    UINT32,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    INT2,
    INT3,
    INT4,
    UINT2,
    UINT3,
    UINT4
};

struct PushConstantsMetadata
{
    struct ResourceMetadata
    {
        ResourceName name;
        bool previousFrame = false;
        bool write = false;
    };

    PushConstantsName name;
    std::vector<ResourceMetadata> resourcesMetadata;
};

}

namespace std 
{

template<>
struct hash<fe::renderer::ShaderMetadata>
{
    std::size_t operator()(const fe::renderer::ShaderMetadata& m) const
    {
        std::size_t seed = 0;
        fe::Utils::hash_combine(seed, m.filePath);
        fe::Utils::hash_combine(seed, static_cast<int>(m.type));
        fe::Utils::hash_combine(seed, static_cast<int>(m.hitGroupType));
        fe::Utils::hash_combine(seed, m.entryPoint);

        for (const std::string& def : m.defines)
            fe::Utils::hash_combine(seed, def);

        return seed;
    }
};

}
