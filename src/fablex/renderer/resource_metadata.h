#pragma once

#include "common.h"
#include "core/window.h"
#include "rhi/resources.h"
#include "core/utils.h"
#include "core/flags_operations.h"

#include <functional>
#include <memory>

namespace fe::renderer
{

enum class ResourceMetadataFlag
{
    UNDEFINED = 0,
    USE_MIPS = 1 << 0,
    TRANSFER_DST = 1 << 1,
    CROSS_FRAME_READ = 1 << 2,
    CROSS_FRAME_READ_NO_HISTORY = 1 << 3,
    PREVIOUS_FRAME = 1 << 4,
    WRITABLE = 1 << 5,
    PING_PONG = 1 << 6,
    PING_PONG_0 = 1 << 7,
    PING_PONG_1 = 1 << 8
};

}

ENABLE_BIT_MASK(fe::renderer::ResourceMetadataFlag);

namespace fe::renderer
{

template<typename NameType>
struct ResourceMetadata
{
    NameType name;
};

using ResourceMetadataHandle = std::unique_ptr<ResourceMetadata<Name>>;

template<typename NameType>
struct ResourceMetadataWithFlags : ResourceMetadata<NameType>
{
    ResourceMetadataFlag flags;

    bool has_flag(ResourceMetadataFlag flag) const
    {
        return ::has_flag(flags, flag);
    }
};

struct TextureMetadata : ResourceMetadataWithFlags<ResourceName>
{
    uint32 layerCount = 1;
    rhi::SampleCount sampleCount = rhi::SampleCount::UNDEFINED;
    rhi::Format format = rhi::Format::UNDEFINED;
};

// Does not inherit from ResourceMetadata because it's not independent metadata
struct RenderTargetMetadata
{
    ResourceName textureName;
    rhi::Format format = rhi::Format::UNDEFINED;
    rhi::StoreOp storeOp = rhi::StoreOp::STORE;
    rhi::LoadOp loadOp = rhi::LoadOp::CLEAR;
    rhi::ClearValues clearValues;
};

struct RenderPassMetadata : ResourceMetadata<RenderPassName>
{
    PipelineName pipelineName;
    RenderPassType type = RenderPassType::GRAPHICS;
    std::vector<ResourceName> inputTextureNames;
    std::vector<RenderTargetMetadata> renderTargetsMetadata;
    std::vector<ResourceName> outputStorageTextureNames;
};

// Does not inherit from ResourceMetadata because it's not independent metadata
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

struct PipelineMetadata : ResourceMetadata<PipelineName>
{
    std::vector<ShaderMetadata> shadersMetadata;
    std::vector<rhi::Format> colorAttachmentFormats;
    rhi::Format depthStencilFormat = rhi::Format::UNDEFINED;
};

struct PushConstantsMetadata : ResourceMetadata<PushConstantsName>
{
    std::vector<ResourceMetadataWithFlags<ResourceName>> resourcesMetadata;
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
