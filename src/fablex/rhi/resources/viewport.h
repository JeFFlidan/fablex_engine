#pragma once

#include "core/types.h"

namespace fe::rhi
{

struct Viewport
{
    uint32 x = 0;
    uint32 y = 0;
    int32 width;
    int32 height;
    uint32 minDepth = 0;
    uint32 maxDepth = 1;
};

struct Scissor
{
    int32 left = 0;
    int32 top = 0;
    int32 right = 0;
    int32 bottom = 0;
};

}