#include "ray_tracing/ray_tracing.hlsli"

PUSH_CONSTANTS(pushConstants, RayTracingPushConstants);

[shader("raygeneration")]
void raygen()
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
    ray.TMax = 10000.0;

    RayPayload payload;

    TraceRay(
        bindlessAccelerationStructures[pushConstants.tlasIndex],
        0,
        ~0,
        0,
        0,
        0,
        ray,
        payload
    );

    bindlessRWTextures2DFloat4[pushConstants.outputTargetIndex][DTid.xy] = payload.color;
}

[shader("closesthit")]
void closest_hit(inout RayPayload payload, in RayAttributes attr)
{
    payload.color = float4(0.4, 0.4, 0.4, 1);
}

[shader("miss")]
void miss(inout RayPayload payload)
{
    payload.color = float4(0.2, 0.3, 0.4, 1.0);
}
