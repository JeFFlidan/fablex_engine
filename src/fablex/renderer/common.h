#pragma once

#include "core/name.h"
#include "core/logger.h"
#include "core/window.h"
#include "rhi/resources.h"

namespace fe::renderer
{

using RenderPassName = Name;
using ResourceName = Name;
using PipelineName = Name;
using FieldName = Name;
using PushConstantsName = Name;

DEFINE_LOG_CATEGORY(LogRenderer);

struct RenderSurface
{
    uint32 width;
    uint32 height;
    rhi::Format renderTargetFormat;
    rhi::Format depthStencilFormat;
    rhi::SwapChain* mainSwapChain;

    uint32 get_window_width() const
    {
        return mainSwapChain->window->get_info().width;
    }

    uint32 get_window_height() const
    {
        return mainSwapChain->window->get_info().height;
    }
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
    struct FieldMetadata
    {
        FieldName name;
        PushConstantType type;
    };

    PushConstantsName name;
    std::vector<FieldMetadata> fieldsMetadata;
};

}