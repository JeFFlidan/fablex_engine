#pragma once

#include "common.h"
#include "core/json_serialization.h"

FE_SERIALIZE_ENUM(fe::renderer::RenderPassType, 
{
    {fe::renderer::RenderPassType::GRAPHICS, "GRAPHICS"},
    {fe::renderer::RenderPassType::ASYNC_COMPUTE, "ASYNC_COMPUTE"}
})

FE_SERIALIZE_ENUM(fe::renderer::PushConstantType, 
{
    {fe::renderer::PushConstantType::FLOAT, "FLOAT"},
    {fe::renderer::PushConstantType::INT32, "INT32"},
    {fe::renderer::PushConstantType::UINT32, "UINT32"},
    {fe::renderer::PushConstantType::FLOAT2, "FLOAT2"},    
    {fe::renderer::PushConstantType::FLOAT3, "FLOAT3"},    
    {fe::renderer::PushConstantType::FLOAT4, "FLOAT4"},
    {fe::renderer::PushConstantType::INT2, "INT2"},
    {fe::renderer::PushConstantType::INT3, "INT3"},
    {fe::renderer::PushConstantType::INT4, "INT4"},
    {fe::renderer::PushConstantType::UINT2, "UINT2"},
    {fe::renderer::PushConstantType::UINT3, "UINT3"},
    {fe::renderer::PushConstantType::UINT4, "UINT4"},
});
