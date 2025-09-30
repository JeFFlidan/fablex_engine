#pragma once

#include "common.h"
#include "resource_metadata.h"
#include "core/json_serialization.h"

FE_SERIALIZE_ENUM(fe::renderer, fe::renderer::RenderPassType, 
{
    {fe::renderer::RenderPassType::GRAPHICS, "GRAPHICS"},
    {fe::renderer::RenderPassType::COMPUTE, "COMPUTE"}
})

FE_SERIALIZE_ENUM(fe::renderer::rg, fe::renderer::rg::ResourceMetadataFlag,
{
    {fe::renderer::rg::ResourceMetadataFlag::UNDEFINED, "Undefined"},
    {fe::renderer::rg::ResourceMetadataFlag::USE_MIPS, "UseMips"},
    {fe::renderer::rg::ResourceMetadataFlag::TRANSFER_DST, "TransferDst"},
    {fe::renderer::rg::ResourceMetadataFlag::CROSS_FRAME_READ, "CrossFrameRead"},
    {fe::renderer::rg::ResourceMetadataFlag::CROSS_FRAME_READ_NO_HISTORY, "CrossFrameReadNoHistory"},
    {fe::renderer::rg::ResourceMetadataFlag::PREVIOUS_FRAME, "PreviousFrame"},
    {fe::renderer::rg::ResourceMetadataFlag::WRITABLE, "Writable"},
    {fe::renderer::rg::ResourceMetadataFlag::PING_PONG, "PingPong"},
    {fe::renderer::rg::ResourceMetadataFlag::PING_PONG_0, "PingPong0"},
    {fe::renderer::rg::ResourceMetadataFlag::PING_PONG_1, "PingPong1"}
})
