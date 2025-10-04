#pragma once

#include "enums.h"
#include <vector>

namespace fe::rhi
{

struct Shader;

struct AssemblyState
{
    TopologyType topologyType = TopologyType::UNDEFINED;
};

struct RasterizationState
{
    PolygonMode polygonMode = PolygonMode::UNDEFINED;
    CullMode cullMode = CullMode::UNDEFINED;
    FrontFace frontFace = FrontFace::UNDEFINED;
    bool isBiasEnabled;
    float lineWidth = 1.0f;
};

struct VertexBindingDescription
{
    uint32_t binding;
    uint32_t stride;
};

struct VertexAttributeDescription
{
    uint32_t binding;
    uint32_t location;
    uint32_t offset;
    Format format = Format::UNDEFINED;
};

struct MultisampleState
{
    SampleCount sampleCount = SampleCount::UNDEFINED;
    bool isEnabled;
};

struct ColorBlendAttachmentState
{
    bool isBlendEnabled;
    BlendFactor srcColorBlendFactor = BlendFactor::UNDEFINED;
    BlendFactor dstColorBlendFactor = BlendFactor::UNDEFINED;
    BlendOp colorBlendOp = BlendOp::UNDEFINED;
    BlendFactor srcAlphaBlendFactor = BlendFactor::UNDEFINED;
    BlendFactor dstAlphaBlendFactor = BlendFactor::UNDEFINED;
    BlendOp alphaBlendOp = BlendOp::UNDEFINED;
    uint64_t colorWriteMask = 0xF;
};

struct ColorBlendState
{
    bool isLogicOpEnabled = false;
    LogicOp logicOp{ LogicOp::UNDEFINED };
    std::vector<ColorBlendAttachmentState> colorBlendAttachments;
};

struct StencilOpState
{
    StencilOp failOp = StencilOp::UNDEFINED;
    StencilOp passOp = StencilOp::UNDEFINED;
    StencilOp depthFailOp = StencilOp::UNDEFINED;
    CompareOp compareOp = CompareOp::UNDEFINED;
    uint32_t compareMask;
    uint32_t writeMask;
    uint32_t reference;
};

struct DepthStencilState
{
    bool isDepthTestEnabled = false;
    bool isDepthWriteEnabled = false;
    CompareOp compareOp = CompareOp::UNDEFINED;
    bool isStencilTestEnabled = false;
    StencilOpState frontStencil;
    StencilOpState backStencil;
};

struct GraphicsPipelineInfo
{
    AssemblyState assemblyState;
    RasterizationState rasterizationState;
    MultisampleState multisampleState;
    ColorBlendState colorBlendState;
    DepthStencilState depthStencilState;
    std::vector<Shader*> shaderStages;
    std::vector<VertexBindingDescription> bindingDescriptions;
    std::vector<VertexAttributeDescription> attributeDescriptions;
    std::vector<Format> colorAttachmentFormats;
    Format depthFormat = Format::UNDEFINED;
};

}