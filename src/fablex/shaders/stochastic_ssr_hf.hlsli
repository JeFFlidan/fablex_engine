#ifndef STOCHASTIC_SSR_HF
#define STOCHASTIC_SSR_HF

#include "rng_hf.hlsli"
#include "surface_hf.hlsli"

static const float GGX_IMPORTANCE_SAMPLE_BIAS = 0.1;

float3x3 get_tangent_basis(float3 tangentZ)
{
    const float sign = tangentZ.z >= 0 ? 1 : -1;
    const float a = -rcp(sign + tangentZ.z);
    const float b = tangentZ.x * tangentZ.y * a;

    float3 tangentX = float3(
        1.0f + sign * a * pow(tangentZ.x, 2),
        sign * b,
        -sign * tangentZ.x
    );

    float3 tangentY = float3(
        b,
        sign + a * pow(tangentZ.y, 2), -tangentZ.y
    );

    return float3x3(tangentX, tangentY, tangentZ);
}

float2 sample_disk(float2 Xi)
{
    float theta = 2 * PI * Xi.x;
    float radius = sqrt(Xi.y);
    return radius * float2(cos(theta), sin(theta));
}

float4 importance_sample_visible_ggx(float2 diskXi, float roughness, float3 V)
{
    roughness = clamp(roughness, MIN_ROUGHNESS, 1.0f);
    float alphaRoughness = roughness * roughness; // App uses alpha roughness by default, maybe I don't need this line. Must test
    float alphaRoughnessSq = alphaRoughness * alphaRoughness;

    float3 Vh = normalize(float3(alphaRoughness * V.xy, V.z));

    float3 tangent0 = (Vh.z < 0.9999) ? normalize(cross(float3(0, 0, 1), Vh)) : float3(1, 0, 0);
    float3 tangent1 = cross(Vh, tangent0);

    float2 p = diskXi;
    float s = 0.5 + 0.5 * Vh.z;
    p.y = (1 - s) * sqrt(1 - p.x * p.x) + s * p.y;

    float3 H;
    H = p.x * tangent0;
    H += p.y * tangent1;
    H += sqrt(saturate(1 - dot(p, p))) * Vh;

    H = normalize(float3(alphaRoughness * H.xy, max(0.0f, H.z)));

    float NoV = V.z;
    float NoH = H.z;
    float VoH = dot(V, H);

    float f = (NoH * alphaRoughnessSq - NoH) * NoH + 1;
    float D = alphaRoughnessSq / (PI * f * f);

    float smithGGXMasking = 2.0f * NoV / (sqrt(NoV * (NoV - NoV * alphaRoughnessSq) + alphaRoughnessSq) + NoV);

    float PDF = smithGGXMasking * VoH * D / NoV;

    return float4(H, PDF);
}

float4 reflection_ggx(in Surface surface, inout RNG rng)
{
    float roughness = clamp(surface.roughness, MIN_ROUGHNESS, 1.0f);
    float4 H;
    float3 L;

    if (roughness > 0.05f)
    {
        float3x3 tangentBasis = get_tangent_basis(surface.N);
        float3 tangentV = mul(tangentBasis, surface.V);
        float2 Xi = rng.next_float2();
        Xi.y = lerp(Xi.y, 0.0f, GGX_IMPORTANCE_SAMPLE_BIAS);
        H = importance_sample_visible_ggx(sample_disk(Xi), roughness, tangentV);
        H.xyz = mul(H.xyz, tangentBasis);
        L = reflect(-surface.V, H.xyz);
    }
    else
    {
        H = float4(surface.N.xyz, 1.0f);
        L = reflect(-surface.V, H.xyz);
    }

    float PDF = H.w;
    return float4(L, PDF);
}

#endif // STOCHASTIC_SSR_HF