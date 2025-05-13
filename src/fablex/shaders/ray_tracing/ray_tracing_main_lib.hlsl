#include "ray_tracing/ray_tracing.hlsli"
#include "common/lighting.hlsli"

PUSH_CONSTANTS(pushConstants, RayTracingPushConstants);

[shader("raygeneration")]
void raygen()
{
    uint2 DTid = DispatchRaysIndex().xy;

    RayDesc ray = get_camera_ray();
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

    uint targetIndex = pushConstants.outputTargetIndex;
    bindlessRWTextures2DFloat4[targetIndex][DTid.xy] = payload.color;
}

[shader("closesthit")]
void closest_hit(inout RayPayload payload, in RayAttributes attr)
{
    PrimitiveInfo primitiveInfo;
    primitiveInfo.primitiveIndex = PrimitiveIndex();
    primitiveInfo.instanceIndex = InstanceIndex();

    Surface surface;
    surface.init(primitiveInfo, attr.barycentrics);
    surface.V = -WorldRayDirection();

    FrameUB frame = get_frame();
    uint lightArrayOffset = frame.lightArrayOffset;
    uint lightArrayCount = frame.lightArrayCount;

    LightingResult lightingResult;
    lightingResult.init(0, 0, 0, 0);

    for (uint i = lightArrayOffset; i != lightArrayOffset + lightArrayCount; ++i)
    {
        ShaderEntity entity = get_entity(i);
        switch (entity.get_type())
        {
        case SHADER_ENTITY_TYPE_POINT_LIGHT:
            break;
        case SHADER_ENTITY_TYPE_SPOT_LIGHT:
            break;
        case SHADER_ENTITY_TYPE_DIRECTIONAL_LIGHT:
            light_directional(entity, surface, lightingResult);
            break;
        }
    }

    payload.color = float4(0.0, 0.0, 0.0, 1.0);
    lightingResult.apply(payload.color);

    // payload.color.xyz = lightingResult.direct.diffuse;
}

[shader("miss")]
void miss(inout RayPayload payload)
{
    payload.color = float4(0.2, 0.3, 0.4, 1.0);
}
