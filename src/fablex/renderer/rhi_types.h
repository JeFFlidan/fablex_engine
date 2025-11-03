#pragma once

#include "rhi/rhi.h"
#include "rhi/resources/enums.h"

namespace fe::renderer
{

using API = rhi::API;
using GPUCapability = rhi::GPUCapability;
using GPUPreference = rhi::GPUPreference;
using GPUType = rhi::GPUType;
using ValidationMode = rhi::ValidationMode;
using ResourceFlags = rhi::ResourceFlags;
using LogicOp = rhi::LogicOp;
using ResourceLayout = rhi::ResourceLayout;
using ResourceUsage = rhi::ResourceUsage;
using MemoryUsage = rhi::MemoryUsage;
using Format = rhi::Format;
using AddressMode = rhi::AddressMode;
using Filter = rhi::Filter;
using SampleCount = rhi::SampleCount;
using TextureDimension = rhi::TextureDimension;
using TextureAspect = rhi::TextureAspect;
using BorderColor = rhi::BorderColor;
using ShaderType = rhi::ShaderType;
using ShaderFormat = rhi::ShaderFormat;
using HLSLShaderModel = rhi::HLSLShaderModel;
using ColorSpace = rhi::ColorSpace;
using ViewType = rhi::ViewType;
using ComponentSwizzle = rhi::ComponentSwizzle;
using QueueType = rhi::QueueType;
using TopologyType = rhi::TopologyType;
using PolygonMode = rhi::PolygonMode;
using CullMode = rhi::CullMode;
using FrontFace = rhi::FrontFace;
using BlendFactor = rhi::BlendFactor;
using BlendOp = rhi::BlendOp;
using CompareOp = rhi::CompareOp;
using StencilOp = rhi::StencilOp;
using PipelineType = rhi::PipelineType;
using LoadOp = rhi::LoadOp;
using StoreOp = rhi::StoreOp;
using RenderingBeginInfoFlags = rhi::RenderingBeginInfoFlags;

using BufferCreateInfo = rhi::BufferInfo;
using BufferViewCreateInfo = rhi::BufferViewInfo;
using TextureCreateInfo = rhi::TextureInfo;
using TextureViewCreateInfo = rhi::TextureViewInfo;
using TextureInitInfo = rhi::TextureInitInfo;
using CommandPoolCreateInfo = rhi::CommandPoolInfo;
using CommandBufferCreateInfo = rhi::CommandBufferInfo;
using SwapChainCreateInfo = rhi::SwapChainInfo;
using ShaderCreateInfo = rhi::ShaderInfo;
using SamplerCreateInfo = rhi::SamplerInfo;
using GraphicsPipelineCreateInfo = rhi::GraphicsPipelineInfo;
using ComputePipelineCreateInfo = rhi::ComputePipelineInfo;
using RayTracingPipelineCreateInfo = rhi::RayTracingPipelineInfo;
using AccelerationStructureCreateInfo = rhi::AccelerationStructureInfo;
using TLASInstance = rhi::TLASInstance;
using Viewport = rhi::Viewport;
using Scissor = rhi::Scissor;
using RenderingBeginInfo = rhi::RenderingBeginInfo;
using PipelineBarrier = rhi::PipelineBarrier;
using DispatchRaysInfo = rhi::DispatchRaysInfo;
using GPUProperties = rhi::GPUProperties;
using ClearValues = rhi::ClearValues;
using MultiviewInfo = rhi::MultiviewInfo;
using AssemblyState = rhi::AssemblyState;
using RasterizationState = rhi::RasterizationState;
using VertexBindingDescription = rhi::VertexBindingDescription;
using VertexAttributeDescription = rhi::VertexAttributeDescription;
using MultisampleState = rhi::MultisampleState;
using ColorBlendAttachmentState = rhi::ColorBlendAttachmentState;
using ColorBlendState = rhi::ColorBlendState;
using StencilOpState = rhi::StencilOpState;
using DepthStencilState = rhi::DepthStencilState;
using MipMap = rhi::MipMap;
using ShaderLibrary = rhi::ShaderLibrary;
using ShaderHitGroup = rhi::ShaderHitGroup;
using ShaderIdentifierBuffer = rhi::ShaderIdentifierBuffer;

}