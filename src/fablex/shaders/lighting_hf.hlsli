#ifndef LIGHTING_HF
#define LIGHTING_HF

#include "common.hlsli"
#include "surface_hf.hlsli"

struct LightingInfo
{
    float3 diffuse;
    float3 specular;
};

struct LightingResult
{
    LightingInfo direct;
    LightingInfo indirect;
};

inline void light_directional_lambert(in ShaderEntity light, in Surface surface, inout LightingResult result)
{
    float3 L = -light.get_direction();

    result.direct.diffuse = max(dot(surface.N, L), 0.00) * light.get_color().xyz * float3(0.8, 0.8, 0.8);
    // result.direct.diffuse = any(result.direct.diffuse) ? result.direct.diffuse : float3(0.1, 0.1, 0.1);
}

#endif