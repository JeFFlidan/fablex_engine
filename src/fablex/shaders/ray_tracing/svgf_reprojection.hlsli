#ifndef SVGF_DEFINE
#define SVGF_DEFINE

#include "ray_tracing/ray_tracing.hlsli"
#include "common/surface.hlsli"
#include "utils/pixel_packing.hlsli"

void svgf_prepare_data(inout PrimaryRayPayload payload, in Surface surface)
{
    uint2 pixel = DispatchRaysIndex().xy;
    ShaderCamera camera = get_camera();

    float2 motionVector = surface.get_motion_vector(get_camera());

    cb.outMotionVector.write(pixel, motionVector);

    float2 normal = ndir_to_oct_snorm(surface.N);
    payload.normalFWidth = length(abs(surface.N_dx) + abs(surface.N_dy));
    if (isnan(payload.normalFWidth)) payload.normalFWidth = 0;

    float depth = distance(surface.P, camera.position);
    if (isnan(depth)) depth = 0;

    float depth_dx = depth - distance(surface.P_quadX, camera.position);
    float depth_dy = depth - distance(surface.P_quadY, camera.position);

    float depthDeriv = max(abs(depth_dx), abs(depth_dy));
    if (isnan(depthDeriv)) depthDeriv = 0;

    cb.outDepthNormal.write(pixel, float4(depth, depthDeriv, normal));
    cb.outAlbedo.write(pixel, float4(surface.baseColor.xyz, 1.0));
    cb.outEmission.write(pixel, float4(surface.emissionColor, 1.0));
}

void svgf_prepare_data(inout PrimaryRayPayload payload)
{
    uint2 pixel = DispatchRaysIndex().xy;
    cb.outMotionVector.write(pixel, 0);
    cb.outDepthNormal.write(pixel, 0);
    cb.outAlbedo.write(pixel, 0);
    cb.outEmission.write(pixel, 0);

    payload.normalFWidth = 0;
}

float3 demodulate(float3 c, float3 albedo)
{
    return c / max(albedo, 0.001);
}

bool is_reprojection_valid(
    int2 coord, 
    float depth, 
    float depthPrev, 
    float depthDeriv, 
    float3 normal, 
    float3 normalPrev, 
    float normalFWidth
)
{
    const int2 dimensions = DispatchRaysDimensions().xy;

    if (any(coord < 1) || any(coord > dimensions - 1))
        return false;

    if (abs(depthPrev - depth) / (depthDeriv + 1e-2) > 10.0)
        return false;

    if (distance(normal, normalPrev) / (normalFWidth + 1e-2) > 16.0)
        return false;

    return true;
}

bool load_previous_data(
    in float normalFWidth,
    in float3 normal,
    in float depth,
    out float4 illuminationPrev, 
    out float2 momentsPrev, 
    out float historyLength
)
{
    const uint2 pixel = DispatchRaysIndex().xy;
    const int2 dimensions = DispatchRaysDimensions().xy;

    const float2 motion = cb.outMotionVector.read(pixel);
    
    // TODO

    illuminationPrev = 0;
    momentsPrev = 0;
    historyLength = 0;

    return true;
}

void svgf_reproject(in float3 color, in float normalFWidth)
{
    const uint2 pixel = DispatchRaysIndex().xy;

    // float3 illumination = demodulate(color - emission, albedo);
    // if (any(any(illumination)))
    //     illumination = 0;
}

#endif // SVGF_DEFINE