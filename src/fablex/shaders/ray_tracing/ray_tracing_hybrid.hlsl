#include "ray_tracing/ray_tracing.hlsli"
#include "common/lighting.hlsli"

PUSH_CONSTANTS(cb, RTHybridLightingPushConstants);

static const uint SHADOW_MISS_INDEX = 0;
static const uint SHADOW_HIT_GROUP_INDEX = 2;

[shader("raygeneration")]
void raygen()
{
    uint2 DTid = DispatchRaysIndex().xy;

    float2 screenResolution = float2(1920, 1080);
    
    float depth = bindlessTextures2DFloat[cb.gBufferDepth.descriptor][DTid.xy];
    if (depth == 0.0f) 
    {
        cb.outColor.write(DTid, float4(0.2, 0.3, 0.4, 1.0)); 
        return;
    }

    Surface surface;

    ShaderCamera camera = get_camera();
    
    float2 pixel = float2(DTid.xy) + 0.5;
    float2 uv = float2(pixel.x / screenResolution.x, pixel.y / screenResolution.y);
    surface.P = reconstruct_position(uv, depth, camera.inverseViewProjection);

    float2 packedNormal = cb.gBufferNormal.read(DTid).xy;
    surface.N = oct_to_ndir_snorm(packedNormal);

    surface.baseColor = cb.gBufferAlbedo.read(DTid);
    float4 materialData = cb.gBufferSurface.read(DTid); 
    surface.roughness = materialData.r;
    surface.metallic = materialData.g;
    
    surface.roughness *= surface.roughness;
    surface.F0 = lerp(float3(0.04f, 0.04f, 0.04f), surface.baseColor.xyz, surface.metallic);
    surface.V = normalize(camera.position.xyz - surface.P);

    FrameUB frame = get_frame();
    uint lightArrayOffset = frame.lightArrayOffset;
    uint lightArrayCount = frame.lightArrayCount;

    LightingResult lightingResult;
    lightingResult.init(0, 0, 0, 0);

    float shadowValue = 1;

    for (uint i = lightArrayOffset; i != lightArrayOffset + lightArrayCount; ++i)
    {
        ShaderEntity entity = get_entity(i);
        switch (entity.get_type())
        {
            case SHADER_ENTITY_TYPE_POINT_LIGHT:
            {
                calculate_point_light(entity, surface, lightingResult);
                break;
            }
            case SHADER_ENTITY_TYPE_SPOT_LIGHT:
            {
                break;
            }
            case SHADER_ENTITY_TYPE_DIRECTIONAL_LIGHT:
            {
                RayDesc shadowRay;
                shadowRay.Origin = surface.P + 0.01f * surface.N; 
                shadowRay.Direction = -entity.get_direction();
                shadowRay.TMin = 0.001f;
                shadowRay.TMax = FLOAT_MAX;

                ShadowRayPayload shadowPayload;
                shadowPayload.rayHitT = FLOAT_MAX;

                TraceRay(
                    cb.tlas.get(),
                    RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
                    ~0,
                    SHADOW_HIT_GROUP_INDEX,
                    0,
                    SHADOW_MISS_INDEX,
                    shadowRay,
                    shadowPayload
                );

                surface.shadow = (shadowPayload.rayHitT < FLOAT_MAX) ? 0.0f : 1.0f;

                light_directional(entity, surface, lightingResult);

                break;
            }
        }
    }

    float4 finalColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    lightingResult.apply(finalColor);
    
    cb.outColor.write(DTid, finalColor);
}

[shader("closesthit")]
void closest_hit_shadow(inout ShadowRayPayload payload, in RayAttributes attr)
{
    payload.rayHitT = RayTCurrent();
}

[shader("miss")]
void miss_shadow(inout ShadowRayPayload payload)
{
    payload.rayHitT = FLOAT_MAX;
}