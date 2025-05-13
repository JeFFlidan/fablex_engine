#ifndef BRDF_D
#define BRDF_D

#include "common/surface.hlsli"

// BRDF functions source: https://google.github.io/filament/Filament.md.html#materialsystem/standardmodel
// For now no optimizations using half instead of float

struct BRDF
{
    float3 L;   // normalized surface to light vector
    float3 H;   // half-vector between view vector and light vector
    float NoL;  // cos angle between normal vector (N) and light vector (L)
    float NoH;  // cos angle between normal vector (N) and half vector (H)
    float NoV;  // cos angle between normal vector (N) and view vector (V)
    float LoH;  // cos angle between light vector (L) and half vector (H)
    float VoH;  // cos angle between view vector (V) and half vector (H)
    float3 F;

    void init(in Surface surface, in float3 Lnormalized)
    {
        L = Lnormalized;
        H = normalize(L + surface.V);

        NoL = saturate(dot(surface.N, L));
        NoH = saturate(dot(surface.N, H));
        NoV = abs(dot(surface.N, surface.V)) + 1e-5;
        LoH = saturate(dot(L, H));
        VoH = saturate(dot(surface.V, H));

        F = f_schlick(VoH, surface.F0);
    }

    float3 specular(in Surface surface)
    {
        float D = d_ggx(surface.roughness);
        float V = v_smith_ggx_correlated(surface.roughness);

        return (D * V) * F * NoL;
    }

    float3 diffuse(in Surface surface)
    {
        float3 diffuse = (1 - F) * (1 - surface.metallic);
        return diffuse * surface.baseColor.xyz * NoL;
    }

    float d_ggx(float roughness)
    {
        float a = NoH * roughness;
        float k = roughness / (1.0 - NoH * NoH + a * a);
        return k * k * (1.0f / PI);
    }

    float v_smith_ggx_correlated(float roughness)
    {
        float a2 = roughness * roughness;
        float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
        float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
        return 0.5 / (GGXV + GGXL);
    }

    float3 f_schlick(float u, float3 F0)
    {
        return F0 + (1.0 - F0) * pow(1.0 - u, 5.0);
    }
};

#endif // BRDF_D