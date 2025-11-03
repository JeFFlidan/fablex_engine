#pragma once

#include "common.h"
#include "rhi_types.h"
#include "core/window.h"
#include "rhi/resources/render_pass.h"
#include "rhi/resources/ray_tracing_pipeline_info.h"
#include "core/utils.h"
#include "core/flags_operations.h"

#include <functional>
#include <memory>

namespace fe::renderer::rg
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

ENABLE_BIT_MASK(fe::renderer::rg::ResourceMetadataFlag);

namespace fe::renderer::rg
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
    SampleCount sampleCount = SampleCount::UNDEFINED;
    Format format = Format::UNDEFINED;
};

// Does not inherit from ResourceMetadata because it's not independent metadata
struct RenderTargetMetadata
{
    ResourceName textureName;
    Format format = Format::UNDEFINED;
    StoreOp storeOp = StoreOp::STORE;
    LoadOp loadOp = LoadOp::CLEAR;
    ClearValues clearValues;
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
    ShaderType type;
    ShaderHitGroup::Type hitGroupType = ShaderHitGroup::TRIANGLES;    // Only for hit shaders
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
    std::vector<Format> colorAttachmentFormats;
    Format depthStencilFormat = Format::UNDEFINED;
};

struct PushConstantsMetadata : ResourceMetadata<PushConstantsName>
{
    std::vector<ResourceMetadataWithFlags<ResourceName>> resourcesMetadata;
};

}

namespace std 
{

template<>
struct hash<fe::renderer::rg::ShaderMetadata>
{
    std::size_t operator()(const fe::renderer::rg::ShaderMetadata& m) const
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
