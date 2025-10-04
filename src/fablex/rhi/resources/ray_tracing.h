#pragma once

#include "internal.h"

namespace fe::rhi
{

class Buffer;

struct ShaderIdentifier
{
    uint64 offset = 0;
    uint64 size = 0;
};

struct ShaderIdentifierBuffer
{
    rhi::Buffer* buffer = nullptr;
    uint64 stride = 0;

    ShaderIdentifier raygenIdentifier;
    ShaderIdentifier missIdentifier;
    ShaderIdentifier hitGroupIdentifier;
    ShaderIdentifier callableIdentifier;
};

struct DispatchRaysInfo
{
    ShaderIdentifierBuffer shaderIdentifierBuffer;

    uint32 width = 1;
    uint32 height = 1;
    uint32 depth = 1;
};

}