#pragma once

#include "core/name.h"
#include "core/logger.h"
#include "rhi/resources.h"

namespace fe::renderer
{

using RenderPassName = Name;
using ResourceName = Name;
using PipelineName = Name;

DEFINE_LOG_CATEGORY(LogRenderer);

struct RenderSurface
{
    uint32 width;
    uint32 height;
    rhi::Format renderTargetFormat;
    rhi::Format depthStencilFormat;
};

struct TextureMetadata
{
    ResourceName textureName;
    uint32 layerCount = 1;
    rhi::SampleCount sampleCount = rhi::SampleCount::UNDEFINED;
    rhi::Format format = rhi::Format::UNDEFINED;
    bool useMips = false;
    bool isTransferDst = false;
};

struct RenderTargetMetadata
{
    ResourceName textureName;
    rhi::Format format = rhi::Format::UNDEFINED;
    rhi::StoreOp storeOp = rhi::StoreOp::STORE;
    rhi::LoadOp loadOp = rhi::LoadOp::DONT_CARE;
    rhi::ClearValues clearValues;
};

struct RenderPassMetadata
{
    RenderPassName renderPassName;
    PipelineName pipelineName;
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
};

struct PipelineMetadata
{
    PipelineName pipelineName;
    std::vector<ShaderMetadata> shadersMetadata;
    std::vector<rhi::Format> colorAttachmentFormats;
    rhi::Format depthStencilFormat = rhi::Format::UNDEFINED;
};

}