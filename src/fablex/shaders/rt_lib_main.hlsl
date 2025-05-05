#include "rt_hf.hlsli"
#include "lighting_hf.hlsli"

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

    FrameUB frame = get_frame();
    uint lightArrayOffset = frame.lightArrayOffset;
    uint lightArrayCount = frame.lightArrayCount;

    LightingResult lightingResult;

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
            light_directional_lambert(entity, surface, lightingResult);
            break;
        }
    }

    payload.color = float4(lightingResult.direct.diffuse, 1.0);
}

[shader("miss")]
void miss(inout RayPayload payload)
{
    payload.color = float4(0.2, 0.3, 0.4, 1.0);
}
