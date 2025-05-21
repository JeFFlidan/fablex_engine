#include "ray_tracing/ray_tracing.hlsli"
#include "common/lighting.hlsli"

PUSH_CONSTANTS(pushConstants, RayTracingPushConstants);

static const uint LIGHT_MISS_INDEX = 0;
static const uint SHADOW_MISS_INDEX = 1;
static const uint LIGHT_HIT_GROUP_INDEX = 2;
static const uint SHADOW_HIT_GROUP_INDEX = 3;

[shader("raygeneration")]
void raygen()
{
    uint2 DTid = DispatchRaysIndex().xy;

    RayDesc ray = get_camera_ray();
    RayPayload payload;

    TraceRay(
        pushConstants.tlas.get(),
        0,
        ~0,
        LIGHT_HIT_GROUP_INDEX,
        0,
        LIGHT_MISS_INDEX,
        ray,
        payload
    );

    pushConstants.outColor.write(DTid.xy, payload.color);
}

[shader("closesthit")]
void closest_hit_light(inout RayPayload payload, in RayAttributes attr)
{
    PrimitiveInfo primitiveInfo;
    primitiveInfo.primitiveIndex = PrimitiveIndex();
    primitiveInfo.instanceIndex = InstanceIndex();
    primitiveInfo.meshIndex = GeometryIndex();

    Surface surface;
    surface.init(primitiveInfo, attr.barycentrics);
    surface.V = -WorldRayDirection();

    FrameUB frame = get_frame();
    uint lightArrayOffset = frame.lightArrayOffset;
    uint lightArrayCount = frame.lightArrayCount;

    LightingResult lightingResult;
    lightingResult.init(0, 0, 0, 0);

    float shadow = 1;

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
        {
            light_directional(entity, surface, lightingResult);

            RayDesc ray = {
                surface.P + 0.01f * surface.N,
                0.001f,
                -entity.get_direction(),
                FLOAT_MAX
            };

            ShadowRayPayload shadowPayload;
            TraceRay(
                pushConstants.tlas.get(),
                RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
                ~0,
                SHADOW_HIT_GROUP_INDEX,
                0,
                SHADOW_MISS_INDEX,
                ray,
                shadowPayload
            );

            shadow = shadowPayload.rayHitT < FLOAT_MAX ? 0 : 1;
            break;
        }
        }
    }

    lightingResult.apply(payload.color);
    // payload.color *= shadow;
}

[shader("closesthit")]
void closest_hit_shadow(inout ShadowRayPayload payload, in RayAttributes attr)
{
    payload.rayHitT = RayTCurrent();
}

[shader("miss")]
void miss_light(inout RayPayload payload)
{
    payload.color = float4(0.2, 0.3, 0.4, 1.0);
}

[shader("miss")]
void miss_shadow(inout ShadowRayPayload payload)
{
    payload.rayHitT = FLOAT_MAX;
}
