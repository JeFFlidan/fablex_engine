#pragma once

#include "common.h"
#include "resource_metadata.h"
#include "core/json_serialization.h"

FE_SERIALIZE_ENUM(fe::renderer, fe::renderer::RenderPassType, 
{
    {fe::renderer::RenderPassType::GRAPHICS, "GRAPHICS"},
    {fe::renderer::RenderPassType::COMPUTE, "COMPUTE"}
})

FE_SERIALIZE_ENUM(fe::renderer, fe::renderer::ResourceMetadataFlag,
{
    {fe::renderer::ResourceMetadataFlag::UNDEFINED, "Undefined"},
    {fe::renderer::ResourceMetadataFlag::USE_MIPS, "UseMips"},
    {fe::renderer::ResourceMetadataFlag::TRANSFER_DST, "TransferDst"},
    {fe::renderer::ResourceMetadataFlag::CROSS_FRAME_READ, "CrossFrameRead"},
    {fe::renderer::ResourceMetadataFlag::CROSS_FRAME_READ_NO_HISTORY, "CrossFrameReadNoHistory"},
    {fe::renderer::ResourceMetadataFlag::PREVIOUS_FRAME, "PreviousFrame"},
    {fe::renderer::ResourceMetadataFlag::WRITABLE, "Writable"},
    {fe::renderer::ResourceMetadataFlag::PING_PONG, "PingPong"},
    {fe::renderer::ResourceMetadataFlag::PING_PONG_0, "PingPong0"},
    {fe::renderer::ResourceMetadataFlag::PING_PONG_1, "PingPong1"}
})
