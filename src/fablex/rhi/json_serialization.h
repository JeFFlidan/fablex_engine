#pragma once

#include "resources.h"
#include "core/json_serialization.h"

FE_SERIALIZE_ENUM(fe::rhi::Format, 
{
    {fe::rhi::Format::UNDEFINED, "UNDEFINED"},
    {fe::rhi::Format::R4G4_UNORM, "R4G4_UNORM"},
    {fe::rhi::Format::R4G4B4A4_UNORM, "R4G4B4A4_UNORM"},
    {fe::rhi::Format::B4G4R4A4_UNORM, "B4G4R4A4_UNORM"},
    {fe::rhi::Format::R5G5B5A1_UNORM, "R5G5B5A1_UNORM"},
    {fe::rhi::Format::B5G5R5A1_UNORM, "B5G5R5A1_UNORM"},
    {fe::rhi::Format::A1R5G5B5_UNORM, "A1R5G5B5_UNORM"},
    {fe::rhi::Format::R8_UNORM, "R8_UNORM"},
    {fe::rhi::Format::R8_SNORM, "R8_SNORM"},
    {fe::rhi::Format::R8_UINT, "R8_UINT"},
    {fe::rhi::Format::R8_SINT, "R8_SINT"},
    {fe::rhi::Format::R8_SRGB, "R8_SRGB"},
    {fe::rhi::Format::R8G8_UNORM, "R8G8_UNORM"},
    {fe::rhi::Format::R8G8_SNORM, "R8G8_SNORM"},
    {fe::rhi::Format::R8G8_UINT, "R8G8_UINT"},
    {fe::rhi::Format::R8G8_SINT, "R8G8_SINT"},
    {fe::rhi::Format::R8G8_SRGB, "R8G8_SRGB"},
    {fe::rhi::Format::R8G8B8A8_UNORM, "R8G8B8A8_UNORM"},
    {fe::rhi::Format::R8G8B8A8_SNORM, "R8G8B8A8_SNORM"},
    {fe::rhi::Format::R8G8B8A8_UINT, "R8G8B8A8_UINT"},
    {fe::rhi::Format::R8G8B8A8_SINT, "R8G8B8A8_SINT"},
    {fe::rhi::Format::R8G8B8A8_SRGB, "R8G8B8A8_SRGB"},
    {fe::rhi::Format::B8G8R8A8_SRGB, "B8G8R8A8_SRGB"},
    {fe::rhi::Format::B8G8R8A8_UNORM, "B8G8R8A8_UNORM"},
    {fe::rhi::Format::B8G8R8A8_SNORM, "B8G8R8A8_SNORM"},
    {fe::rhi::Format::R10G10B10A2_UNORM, "R10G10B10A2_UNORM"},
    {fe::rhi::Format::R10G10B10A2_SNORM, "R10G10B10A2_SNORM"},
    {fe::rhi::Format::R16_UNORM, "R16_UNORM"},
    {fe::rhi::Format::R16_SNORM, "R16_SNORM"},
    {fe::rhi::Format::R16_UINT, "R16_UINT"},
    {fe::rhi::Format::R16_SINT, "R16_SINT"},
    {fe::rhi::Format::R16_SFLOAT, "R16_SFLOAT"},
    {fe::rhi::Format::R16G16_UNORM, "R16G16_UNORM"},
    {fe::rhi::Format::R16G16_SNORM, "R16G16_SNORM"},
    {fe::rhi::Format::R16G16_UINT, "R16G16_UINT"},
    {fe::rhi::Format::R16G16_SINT, "R16G16_SINT"},
    {fe::rhi::Format::R16G16_SFLOAT, "R16G16_SFLOAT"},
    {fe::rhi::Format::R16G16B16A16_UNORM, "R16G16B16A16_UNORM"},
    {fe::rhi::Format::R16G16B16A16_SNORM, "R16G16B16A16_SNORM"},
    {fe::rhi::Format::R16G16B16A16_UINT, "R16G16B16A16_UINT"},
    {fe::rhi::Format::R16G16B16A16_SINT, "R16G16B16A16_SINT"},
    {fe::rhi::Format::R16G16B16A16_SFLOAT, "R16G16B16A16_SFLOAT"},
    {fe::rhi::Format::R32_UINT, "R32_UINT"},
    {fe::rhi::Format::R32_SINT, "R32_SINT"},
    {fe::rhi::Format::R32_SFLOAT, "R32_SFLOAT"},
    {fe::rhi::Format::R32G32_UINT, "R32G32_UINT"},
    {fe::rhi::Format::R32G32_SINT, "R32G32_SINT"},
    {fe::rhi::Format::R32G32_SFLOAT, "R32G32_SFLOAT"},
    {fe::rhi::Format::R32G32B32_UINT, "R32G32B32_UINT"},
    {fe::rhi::Format::R32G32B32_SINT, "R32G32B32_SINT"},
    {fe::rhi::Format::R32G32B32_SFLOAT, "R32G32B32_SFLOAT"},
    {fe::rhi::Format::R32G32B32A32_UINT, "R32G32B32A32_UINT"},
    {fe::rhi::Format::R32G32B32A32_SINT, "R32G32B32A32_SINT"},
    {fe::rhi::Format::R32G32B32A32_SFLOAT, "R32G32B32A32_SFLOAT"},
    {fe::rhi::Format::D16_UNORM, "D16_UNORM"},
    {fe::rhi::Format::D32_SFLOAT, "D32_SFLOAT"},
    {fe::rhi::Format::S8_UINT, "S8_UINT"},
    {fe::rhi::Format::D16_UNORM_S8_UINT, "D16_UNORM_S8_UINT"},
    {fe::rhi::Format::D24_UNORM_S8_UINT, "D24_UNORM_S8_UINT"},
    {fe::rhi::Format::D32_SFLOAT_S8_UINT, "D32_SFLOAT_S8_UINT"},
    {fe::rhi::Format::BC1_RGBA_UNORM, "BC1_RGBA_UNORM"},
    {fe::rhi::Format::BC1_RGBA_SRGB_UNORM, "BC1_RGBA_SRGB_UNORM"},
    {fe::rhi::Format::BC2_UNORM, "BC2_UNORM"},
    {fe::rhi::Format::BC2_SRGB, "BC2_SRGB"},
    {fe::rhi::Format::BC3_UNORM, "BC3_UNORM"},
    {fe::rhi::Format::BC3_SRGB, "BC3_SRGB"},
    {fe::rhi::Format::BC4_UNORM, "BC4_UNORM"},
    {fe::rhi::Format::BC4_SNORM, "BC4_SNORM"},
    {fe::rhi::Format::BC5_UNORM, "BC5_UNORM"},
    {fe::rhi::Format::BC5_SNORM, "BC5_SNORM"},
    {fe::rhi::Format::BC6H_UFLOAT, "BC6H_UFLOAT"},
    {fe::rhi::Format::BC6H_SFLOAT, "BC6H_SFLOAT"},
    {fe::rhi::Format::BC7_UNORM, "BC7_UNORM"},
    {fe::rhi::Format::BC7_SRGB, "BC7_SRGB"}
})

FE_SERIALIZE_ENUM(fe::rhi::AddressMode, 
{
    {fe::rhi::AddressMode::REPEAT, "REPEAT"},
    {fe::rhi::AddressMode::MIRRORED_REPEAT, "MIRRORED_REPEAT"},
    {fe::rhi::AddressMode::CLAMP_TO_EDGE, "CLAMP_TO_EDGE"},
    {fe::rhi::AddressMode::CLAMP_TO_BORDER, "CLAMP_TO_BORDER"},
    {fe::rhi::AddressMode::MIRROR_CLAMP_TO_EDGE, "MIRROR_CLAMP_TO_EDGE"}    
})

FE_SERIALIZE_ENUM(fe::rhi::LoadOp, 
{
    {fe::rhi::LoadOp::DONT_CARE, "DONT_CARE"},
    {fe::rhi::LoadOp::LOAD, "LOAD"},
    {fe::rhi::LoadOp::CLEAR, "CLEAR"}
})

FE_SERIALIZE_ENUM(fe::rhi::StoreOp, 
{
    {fe::rhi::StoreOp::STORE, "STORE"},
    {fe::rhi::StoreOp::DONT_CARE, "DONT_CARE"}
})

FE_SERIALIZE_ENUM(fe::rhi::ShaderType, 
{
    {fe::rhi::ShaderType::VERTEX, "VERTEX"},
    {fe::rhi::ShaderType::FRAGMENT, "FRAGMENT"},
    {fe::rhi::ShaderType::TESSELLATION_CONTROL, "TESSELLATION_CONTROL"},
    {fe::rhi::ShaderType::TESSELLATION_EVALUATION, "TESSELLATION_EVALUATION"},
    {fe::rhi::ShaderType::GEOMETRY, "GEOMETRY"},
    {fe::rhi::ShaderType::COMPUTE, "COMPUTE"},
    {fe::rhi::ShaderType::MESH, "MESH"},
    {fe::rhi::ShaderType::TASK, "TASK"},
    {fe::rhi::ShaderType::RAY_GENERATION, "RAY_GENERATION"},
    {fe::rhi::ShaderType::RAY_INTERSECTION, "RAY_INTERSECTION"},
    {fe::rhi::ShaderType::RAY_ANY_HIT, "RAY_ANY_HIT"},
    {fe::rhi::ShaderType::RAY_CLOSEST_HIT, "RAY_CLOSEST_HIT"},
    {fe::rhi::ShaderType::RAY_MISS, "RAY_MISS"},
    {fe::rhi::ShaderType::RAY_CALLABLE, "RAY_CALLABLE"},
    {fe::rhi::ShaderType::LIB, "LIB"}
})

FE_SERIALIZE_ENUM(fe::rhi::PipelineType, 
{
    {fe::rhi::PipelineType::COMPUTE, "COMPUTE"},
    {fe::rhi::PipelineType::GRAPHICS, "GRAPHICS"},
    {fe::rhi::PipelineType::RAY_TRACING, "RAY_TRACING"},
    {fe::rhi::PipelineType::UNDEFINED, "UNDEFINED"}
})
