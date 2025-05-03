#ifndef RT_HF
#define RT_HF

#include "common.hlsli"
#include "shader_interop_renderer.h"

struct [raypayload] RayPayload
{
    float4 color : read(caller) : write(closesthit, miss);
};

typedef BuiltInTriangleIntersectionAttributes RayAttributes;

bool is_inside_viewport(float2 p, Viewport viewport)
{
    return p.x >= viewport.left && p.x <= viewport.right
        && p.y >= viewport.top && p.y <= viewport.bottom;
}

#endif // RT_HF