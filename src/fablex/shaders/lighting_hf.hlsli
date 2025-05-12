#ifndef LIGHTING_HF
#define LIGHTING_HF

#include "common.hlsli"
#include "surface_hf.hlsli"
#include "brdf_hf.hlsli"

struct LightingInfo
{
    float3 diffuse;
    float3 specular;
};

struct LightingResult
{
    LightingInfo direct;
    LightingInfo indirect;

    void init(
        float3 directDiffuse,
        float3 directSpecular,
        float3 indirectDiffuse,
        float3 indirectSpecular
    )
    {
        direct.diffuse = directDiffuse;
        direct.specular = directSpecular;
        indirect.diffuse = indirectDiffuse;
        indirect.specular = indirectSpecular;
    }

    void apply(inout float4 color)
    {
        // TODO: Add indirect lighting
        float3 diffuse = direct.diffuse / PI;
        float3 specular = direct.specular;
        color.rgb = diffuse;
        color.rgb += specular;
    }

    float4 apply()
    {
        float4 color = float4(0, 0, 0, 1);
        float3 diffuse = direct.diffuse / PI;
        float3 specular = direct.specular;
        color.rgb = diffuse;
        color.rgb += specular;
        return color;
    }
};

inline void light_directional_lambert(in ShaderEntity light, in Surface surface, inout LightingResult result)
{
    float3 L = -light.get_direction();

    result.direct.diffuse = max(dot(surface.N, L), 0.00) * light.get_color().xyz * float3(0.8, 0.8, 0.8);
    // result.direct.diffuse = any(result.direct.diffuse) ? result.direct.diffuse : float3(0.1, 0.1, 0.1);
}

inline void light_directional(in ShaderEntity light, inout Surface surface, inout LightingResult result)
{
    float3 L = -light.get_direction();
    float3 lightColor = light.get_color().xyz;
    
    BRDF brdf;
    brdf.init(surface, L);

    surface.F = brdf.F;
    result.direct.diffuse = mad(lightColor, brdf.diffuse(surface), result.direct.diffuse);
    result.direct.specular = mad(lightColor, brdf.specular(surface), result.direct.specular);
}

#endif