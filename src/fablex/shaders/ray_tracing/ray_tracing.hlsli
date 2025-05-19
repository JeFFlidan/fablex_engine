#ifndef RT
#define RT

#include "common.hlsli"
#include "shader_interop_renderer.h"

struct [raypayload] RayPayload
{
    float4 color : read(caller) : write(closesthit, miss);
};

struct [raypayload] PrimaryRayPayload
{
    uint rng : read(caller, closesthit, miss) : write(caller, closesthit, miss);
    float3 energy : read(caller, closesthit) : write(caller, closesthit);
    float3 rayDirection : read(caller, closesthit) : write(caller, closesthit);
    float3 rayOrigin : read(caller, closesthit) : write(caller, closesthit);
    float3 color : read(caller) : write(closesthit, miss);
    float normalFWidth : read(caller) : write(closesthit, miss);
};

struct [raypayload] SecondaryRayPayload
{
    uint rng : read(caller, closesthit, miss) : write(caller, closesthit, miss);
    float3 energy : read(caller, closesthit) : write(caller, closesthit);
    float3 rayDirection : read(caller, closesthit) : write(caller, closesthit);
    float3 rayOrigin : read(caller, closesthit) : write(caller, closesthit);
    float3 color : read(caller) : write(closesthit, miss);
};

struct [raypayload] ShadowRayPayload
{
    float rayHitT : read(caller) : write(miss, closesthit);
};

typedef BuiltInTriangleIntersectionAttributes RayAttributes;

bool is_inside_viewport(float2 p, Viewport viewport)
{
    return p.x >= viewport.left && p.x <= viewport.right
        && p.y >= viewport.top && p.y <= viewport.bottom;
}

RayDesc get_camera_ray()
{
    uint2 DTid = DispatchRaysIndex().xy;
    float2 dimensions = (float2)DispatchRaysDimensions();

    const float2 uv = ((float2)DTid.xy + 0.5) / dimensions;
    const float2 ndc = 2.0 * float2(uv.x, 1.0 - uv.y) - 1.0;
    const float4 clip = float4(ndc, 0, 1.0);

    ShaderCamera camera = get_camera();
    float4x4 invVP = mul(camera.inverseView, camera.inverseProjection);
    float4 worldPos = mul(invVP, clip);
    worldPos /= worldPos.w;

    float3 rayOrigin = camera.position;
    float3 rayDir = normalize(worldPos.xyz - rayOrigin);

    RayDesc ray;
    ray.Origin = rayOrigin;
    ray.Direction = rayDir;
    ray.TMin = 0.001;
    ray.TMax = FLOAT_MAX;

    return ray;
}

#endif // RT