
#include "common.hlsli"

PUSH_CONSTANTS(cb, PathTracingPushConstants);

#define SURFACE_NORMAL_DERIVATIVES
#define SURFACE_POSITION_DERIVATIVES

#include "ray_tracing/ray_tracing.hlsli"
#include "ray_tracing/stochastic_ssr.hlsli"
#include "ray_tracing/svgf_reprojection.hlsli"

#include "common/brdf.hlsli"
#include "common/lighting.hlsli"

static const uint PRIMARY_LIGHT_MISS_INDEX = 0;
static const uint SECONDARY_LIGHT_MISS_INDEX = 1;
static const uint SHADOW_MISS_INDEX = 2;
static const uint PRIMARY_LIGHT_HIT_GROUP_INDEX = 4;
static const uint SECONDARY_LIGHT_HIT_GROUP_INDEX = 5;
static const uint SHADOW_HIT_GROUP_INDEX = 6;

float3 exec_closest_hit(
    in float2 barycentrics,
    inout uint rngState,
    inout float3 energy,
    out float3 rayOrigin,
    out float3 rayDirection,
    out Surface surface
)
{
    RNG rng = (RNG)rngState;

    PrimitiveInfo primitiveInfo;
    primitiveInfo.primitiveIndex = PrimitiveIndex();
    primitiveInfo.instanceIndex = InstanceIndex();

    surface.init(primitiveInfo, barycentrics, WorldRayOrigin(), WorldRayDirection());
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
        cb.tlas.get(),
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
        ~0,
        SHADOW_HIT_GROUP_INDEX,
        1,
        SHADOW_MISS_INDEX,
        ray,
        shadowPayload
    );

    float shadow = shadowPayload.rayHitT < FLOAT_MAX ? 0 : 1;

    float3 color = energy * shadow * iter.count * mad(surface.baseColor.xyz / PI, lighting.direct.diffuse, lighting.direct.specular);

    const float specularChance = dot(surface.F, 0.333f);
    if (rng.next_float() < specularChance)
    {
        rayDirection = reflection_ggx(surface, rng).xyz;
        energy = surface.F / max(0.001, specularChance);
    }
    else
    {
        rayDirection = sample_hemisphere_cos(surface.N, rng);
        energy = surface.baseColor.xyz * (1 - surface.F) / max(0.01, 1 - specularChance);
    }

    rayOrigin = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();

    if (dot(rayDirection, surface.N) <= 0)
        rayOrigin += surface.N * 0.001;

    rngState = (uint)rng;

    return color;
}

bool prepare_next_bounce(
    inout RNG rng,
    inout float3 rayOrigin,
    inout float3 rayDirection,
    out RayDesc ray,
    inout float3 payloadEnergy,
    inout float3 energy,
    in float3 color,
    inout float3 result,
    bool isPrimary
)
{
    if (!any(rayDirection) && !any(rayOrigin))
    {
        if (isPrimary)
            result = color;
        return false;
    }

    result += color;

    // Russian Roulette, https://computergraphics.stackexchange.com/questions/2316/is-russian-roulette-really-the-answer 
    const float terminationChance = max3(energy);
    if (rng.next_float() > terminationChance)
        return false;
    energy /= terminationChance;

    energy *= payloadEnergy;
    payloadEnergy = energy;

    ray.Origin = rayOrigin;
    ray.Direction = rayDirection;
    ray.TMin = 0.001;
    ray.TMax = FLOAT_MAX;

    rayOrigin = 0;
    rayDirection = 0;

    return true;
}

[shader("raygeneration")]
void raygen()
{
    uint bounces = cb.bounceCount;
    float3 result = 0;
    float3 energy = 1;

    uint2 pixel = DispatchRaysIndex().xy;
    uint2 dimensions = DispatchRaysDimensions().xy;
    uint id = pixel.x + dimensions.x * pixel.y;

    RNG rng;
    rng.compute_rng_seed(id, cb.frameNumber, bounces);

    RayDesc ray = get_camera_ray();

    PrimaryRayPayload primaryPayload;
    primaryPayload.energy = 1;
    primaryPayload.rayDirection = 0;
    primaryPayload.rayOrigin = 0;
    primaryPayload.rng = (uint)rng;

    TraceRay(cb.tlas.get(), 0, ~0, PRIMARY_LIGHT_HIT_GROUP_INDEX, 0, PRIMARY_LIGHT_MISS_INDEX, ray, primaryPayload);

    rng = (RNG)primaryPayload.rng;

    bool canExecNextBounce = prepare_next_bounce(
        rng,
        primaryPayload.rayOrigin,
        primaryPayload.rayDirection,
        ray,
        primaryPayload.energy,
        energy,
        primaryPayload.color,
        result,
        true
    );

    if (canExecNextBounce)
    {
        SecondaryRayPayload secondaryPayload;
        secondaryPayload.energy = energy;
        secondaryPayload.rayDirection = 0;
        secondaryPayload.rayOrigin = 0;

        for (uint bounce = 0; bounce != bounces - 1; ++bounce)
        {
            secondaryPayload.rng = (uint)rng;
            TraceRay(cb.tlas.get(), 0, ~0, SECONDARY_LIGHT_HIT_GROUP_INDEX, 1, SECONDARY_LIGHT_MISS_INDEX, ray, secondaryPayload);
            rng = (RNG)secondaryPayload.rng;

            canExecNextBounce = prepare_next_bounce(
                rng,
                secondaryPayload.rayOrigin,
                secondaryPayload.rayDirection,
                ray,
                secondaryPayload.energy,
                energy,
                secondaryPayload.color,
                result,
                false
            );

            if (!canExecNextBounce)
                break;
        }
    }

    float4 oldValue = cb.inPrevIllumination.read(pixel);
    cb.outIllumination.write(pixel, lerp(oldValue, float4(result, 1), cb.accumulationFactor));  // FOR TEST ONLY
    // cb.outIllumination.write(pixel, 1);  // FOR TEST ONLY

    svgf_reproject(result, primaryPayload.normalFWidth);
}

[shader("closesthit")]
void closest_hit_primary(inout PrimaryRayPayload payload, in RayAttributes attr)
{
    Surface surface;
    payload.color = exec_closest_hit(
        attr.barycentrics,
        payload.rng,
        payload.energy,
        payload.rayOrigin,
        payload.rayDirection,
        surface
    );
    svgf_prepare_data(payload, surface);
}

[shader("closesthit")]
void closest_hit_secondary(inout SecondaryRayPayload payload, in RayAttributes attr)
{
    Surface surface;
    payload.color = exec_closest_hit(
        attr.barycentrics,
        payload.rng,
        payload.energy,
        payload.rayOrigin,
        payload.rayDirection,
        surface
    );
}

[shader("closesthit")]
void closest_hit_shadow(inout PTShadowRayPayload payload, in RayAttributes attr)
{
    payload.rayHitT = RayTCurrent();
}

[shader("miss")]
void miss_primary(inout PrimaryRayPayload payload)
{
    payload.color = float3(0.4, 0.4, 0.4);
    svgf_prepare_data(payload);
}

[shader("miss")]
void miss_secondary(inout SecondaryRayPayload payload)
{
    payload.color = float3(0.4, 0.4, 0.4);
}

[shader("miss")]
void miss_shadow(inout PTShadowRayPayload payload)
{
    payload.rayHitT = FLOAT_MAX;
}
