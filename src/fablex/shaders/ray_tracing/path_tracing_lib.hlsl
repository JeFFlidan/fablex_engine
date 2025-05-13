#include "ray_tracing/ray_tracing.hlsli"
#include "ray_tracing/stochastic_ssr.hlsli"

#include "common.hlsli"
#include "common/brdf.hlsli"
#include "common/lighting.hlsli"

PUSH_CONSTANTS(pushConstants, PathTracingPushConstants);

static const uint LIGHT_HIT_GROUP_INDEX = 2;
static const uint SHADOW_HIT_GROUP_INDEX = 3;
static const uint LIGHT_MISS_INDEX = 0;
static const uint SHADOW_MISS_INDEX = 1;

struct [raypayload] PTRayPayload
{
    uint rng : read(caller, closesthit, miss) : write(caller, closesthit, miss);
    float3 energy : read(caller, closesthit) : write(caller, closesthit);
    float3 rayDirection : read(caller, closesthit) : write(caller, closesthit);
    float3 rayOrigin : read(caller, closesthit) : write(caller, closesthit);
    float3 color : read(caller) : write(closesthit, miss);
    bool isPrimaryHit : read(closesthit, miss) : write(caller);
};

struct [raypayload] PTShadowRayPayload
{
    float rayHitT : read(caller) : write(miss, closesthit);
};

void write_to_color(float4 value)
{
    bindlessRWTextures2DFloat4[pushConstants.outputTextureIndex][DispatchRaysIndex().xy] = value;
}

float4 read_from_color()
{
    return bindlessRWTextures2DFloat4[pushConstants.outputTextureIndex][DispatchRaysIndex().xy];
}

void write_to_motion_vector(float2 value)
{
    bindlessRWTextures2DFloat2[pushConstants.motionVectorTextureIndex][DispatchRaysIndex().xy] = value;
}

float2 read_from_motion_vector()
{
    return bindlessRWTextures2DFloat2[pushConstants.motionVectorTextureIndex][DispatchRaysIndex().xy];   
}

RaytracingAccelerationStructure get_as()
{
    return bindlessAccelerationStructures[pushConstants.tlasIndex];
}

[shader("raygeneration")]
void raygen()
{
    uint bounces = pushConstants.bounceCount;
    float3 result = 0;

    uint2 pixel = DispatchRaysIndex().xy;
    uint2 dimensions = DispatchRaysDimensions().xy;
    uint id = pixel.x + dimensions.x * pixel.y;

    RNG rng;
    rng.compute_rng_seed(id, pushConstants.frameNumber, bounces);

    PTRayPayload payload;
    payload.energy = 1;
    payload.rayDirection = 0;
    payload.rayOrigin = 0;
    payload.isPrimaryHit = true;

    float3 energy = 1;

    RayDesc ray = get_camera_ray();

    for (uint bounce = 0; bounce != bounces; ++bounce)
    {
        payload.rng = (uint)rng;
        TraceRay(get_as(), 0, ~0, LIGHT_HIT_GROUP_INDEX, 1, LIGHT_MISS_INDEX, ray, payload);
        rng = (RNG)payload.rng;

        float3 rayOrigin = payload.rayOrigin;
        float3 rayDirection = payload.rayDirection;
        float3 newEnergy = payload.energy;
        float3 color = payload.color;

        if (!any(rayDirection) && !any(rayOrigin))
        {
            if (bounce == 0)
                result = color;
            break;
        }

        result += color;

        // Russian Roulette, https://computergraphics.stackexchange.com/questions/2316/is-russian-roulette-really-the-answer 
        const float terminationChance = max3(energy);
        if (rng.next_float() > terminationChance)
            break;
        energy /= terminationChance;

        energy *= newEnergy;
        payload.energy = energy;

        ray.Origin = payload.rayOrigin;
        ray.Direction = payload.rayDirection;
        ray.TMin = 0.001;
        ray.TMax = FLOAT_MAX;

        payload.rayOrigin = 0;
        payload.rayDirection = 0;
        payload.isPrimaryHit = false;
    }

    float4 oldValue = read_from_color();
    write_to_color(lerp(oldValue, float4(result, 1), pushConstants.accumulationFactor));
}

[shader("closesthit")]
void closest_hit(inout PTRayPayload payload, in RayAttributes attr)
{
    RNG rng = (RNG)payload.rng;

    PrimitiveInfo primitiveInfo;
    primitiveInfo.primitiveIndex = PrimitiveIndex();
    primitiveInfo.instanceIndex = InstanceIndex();

    Surface surface;
    surface.init(primitiveInfo, attr.barycentrics);
    surface.V = -WorldRayDirection();

    ShaderEntityIterator iter = lights_iter();
    ShaderEntity light = get_entity(iter.random_item(rng));

    LightingResult lighting;
    lighting.init(0, 0, 0, 0);

    switch (light.get_type())
    {
    case SHADER_ENTITY_TYPE_POINT_LIGHT:
        break;
    case SHADER_ENTITY_TYPE_SPOT_LIGHT:
        break;
    case SHADER_ENTITY_TYPE_DIRECTIONAL_LIGHT:
        light_directional(light, surface, lighting);
        break;
    }

    RayDesc ray = {
        surface.P + 0.01f * surface.N,
        0.001f,
        -light.get_direction(),
        FLOAT_MAX
    };

    PTShadowRayPayload shadowPayload;
    TraceRay(
        get_as(),
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
        ~0,
        SHADOW_HIT_GROUP_INDEX,
        1,
        SHADOW_MISS_INDEX,
        ray,
        shadowPayload
    );

    float shadow = shadowPayload.rayHitT < FLOAT_MAX ? 0 : 1;

    payload.color = payload.energy * shadow * iter.count * mad(surface.baseColor.xyz / PI, lighting.direct.diffuse, lighting.direct.specular);

    const float specularChance = dot(surface.F, 0.333f);
    if (rng.next_float() < specularChance)
    {
        payload.rayDirection = reflection_ggx(surface, rng).xyz;
        payload.energy = surface.F / max(0.001, specularChance);
    }
    else
    {
        payload.rayDirection = sample_hemisphere_cos(surface.N, rng);
        payload.energy = surface.baseColor.xyz * (1 - surface.F) / max(0.01, 1 - specularChance);
    }

    if (dot(payload.rayDirection, surface.N) <= 0)
        payload.rayOrigin += surface.N * 0.001;

    payload.rayOrigin = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
    payload.rng = (uint)rng;

    if (!payload.isPrimaryHit)
        return;

    write_to_motion_vector(surface.get_motion_vector(get_camera()));
}

[shader("closesthit")]
void closest_hit_shadow(inout PTShadowRayPayload payload, in RayAttributes attr)
{
    payload.rayHitT = RayTCurrent();
}

[shader("miss")]
void miss(inout PTRayPayload payload)
{
    payload.color = float3(0.4, 0.4, 0.4);

    if (!payload.isPrimaryHit)
        return;

    write_to_motion_vector(0);
}

[shader("miss")]
void miss_shadow(inout PTShadowRayPayload payload)
{
    payload.rayHitT = FLOAT_MAX;
}
