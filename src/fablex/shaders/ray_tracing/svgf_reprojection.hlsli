#ifndef SVGF_REPROJECTION
#define SVGF_REPROJECTION

#include "ray_tracing/svgf_common.hlsli"
#include "ray_tracing/ray_tracing.hlsli"
#include "common/surface.hlsli"
#include "utils/pixel_packing.hlsli"

void svgf_prepare_data(inout PrimaryRayPayload payload, in Surface surface)
{
    uint2 pixel = DispatchRaysIndex().xy;
    ShaderCamera camera = get_camera();

    float2 motionVector = surface.get_motion_vector(get_camera());

    cb.outMotionVector.write(pixel, motionVector);

    float2 normal = ndir_to_oct_snorm(surface.N);
    payload.normalFWidth = length(abs(surface.N_dx) + abs(surface.N_dy));
    if (isnan(payload.normalFWidth)) payload.normalFWidth = 0;

    // float depth = distance(surface.P, camera.position);
    float4 temp = mul(camera.viewProjection, float4(surface.P, 1.0));
    float depth = temp.z * temp.w;
    if (isnan(depth)) depth = 0;

    temp = mul(camera.viewProjection, float4(surface.P_quadX, 1));
    float depth_dx = depth - (temp.z * temp.w);

    temp = mul(camera.viewProjection, float4(surface.P_quadY, 1));
    float depth_dy = depth - (temp.z * temp.w);

    float depthDeriv = max(abs(depth_dx), abs(depth_dy));
    if (isnan(depthDeriv)) depthDeriv = 0;

    cb.outDepthNormal.write(pixel, float4(depth, depthDeriv, normal));
    cb.outAlbedo.write(pixel, float4(surface.baseColor.xyz, 1.0));
    cb.outEmission.write(pixel, float4(surface.emissionColor, 1));
}

void svgf_prepare_data(inout PrimaryRayPayload payload)
{
    uint2 pixel = DispatchRaysIndex().xy;
    cb.outMotionVector.write(pixel, 0);
    cb.outDepthNormal.write(pixel, 0);
    cb.outAlbedo.write(pixel, 0);
    cb.outEmission.write(pixel, 0);
    cb.outIllumination.write(pixel, 0);
    cb.outMoments.write(pixel, 0);
    cb.outHistoryLength.write(pixel, 1);

    payload.normalFWidth = 0;
}

float3 demodulate(float3 c, float3 albedo)
{
    return c / max(albedo, 0.001);
}

bool is_reprojection_valid(
    int2 coord, 
    float depth, 
    float prevDepth, 
    float depthDeriv, 
    float3 normal, 
    float3 prevNormal, 
    float normalFWidth
)
{
    const int2 dimensions = (uint2)DispatchRaysDimensions().xy;
    
    if (any(coord < 1) || any(coord > dimensions - 1))
        return false;
    cb.outColor.write(coord, float4(abs(prevDepth - depth) / (depthDeriv + 1e-2f) > 10.f, distance(prevNormal, normal) / (normalFWidth + 1e-2) > 16.0, abs(prevDepth - depth) / (depthDeriv + 1e-2f), 1));
    const int prevModelInstance = (int)cb.outEmission[coord].a;

    if (abs(prevDepth - depth) / (depthDeriv + 1e-2f) > 10.f)
        return false;

    if (distance(prevNormal, normal) / (normalFWidth + 1e-2) > 16.0)
        return false;

    return true;
}

bool load_previous_data(
    in float normalFWidth,
    in float3 normal,
    in float2 depth,
    out float4 prevIllumination, 
    out float2 prevMoments, 
    out float historyLength
)
{
    const uint2 pixel = DispatchRaysIndex().xy;
    const float2 dimensions = DispatchRaysDimensions().xy;

    const float2 motion = cb.outMotionVector.read(pixel) * dimensions;

    const int2 prevPixel = int2(float2(pixel) + motion);
    // const int2 prevPixel = pixel;

    prevIllumination = 0;
    prevMoments = 0;

    bool v[4];
    const float2 prevPos = float2(pixel) + motion;
    const uint2 offset[4] = {uint2(0, 0), uint2(1, 0), uint2(0, 1), uint2(1, 1)};

    bool valid = false;
    for (int sampleIdx = 0; sampleIdx < 4; ++sampleIdx)
    {
        int2 loc = prevPixel + offset[sampleIdx];
        float2 prevDepth = cb.inPrevDepthNormal[loc].xy;
        float3 prevNormal = oct_to_ndir_snorm(cb.inPrevDepthNormal[loc].zw);

        v[sampleIdx] = is_reprojection_valid(prevPos, depth.x, prevDepth.x, depth.y, normal, prevNormal, normalFWidth);

        valid = valid || v[sampleIdx];
    }

    if (valid)
    {
        float sumw = 0;
        float x = frac(prevPos.x);
        float y = frac(prevPos.y);

        const float w[4] = {(1 - x) * (1 - y), x * (1 - y), (1 - x) * y, x * y};

        for (int sampleIdx = 0; sampleIdx < 4; ++sampleIdx)
        {
            const uint2 loc = uint2(prevPos) + offset[sampleIdx];
            if (v[sampleIdx])
            {
                prevIllumination += w[sampleIdx] * cb.inPrevIllumination.get()[loc];
                prevMoments += w[sampleIdx] * cb.inPrevMoments.get()[loc].xy;
                sumw += w[sampleIdx];
            }
        }
        valid = (sumw >= 0.01);
        prevIllumination = valid ? prevIllumination / sumw : 0;
        prevMoments = valid ? prevMoments / sumw : 0;
        cb.outEmission.write(pixel, float4(prevIllumination.xyz, sumw));    // DEBUG
    }

    // if (!valid)
    // {
    //     float nValid = 0.0;

    //     const int radius = 1;
    //     for (int yy = -radius; yy <= radius; ++yy)
    //     {
    //         for (int xx = -radius; xx <= radius; ++xx)
    //         {
    //             const uint2 p = uint2(int2(prevPixel) + int2(xx, yy));
    //             const float2 depthFilter = cb.inPrevDepthNormal[p].xy;
    //             const float3 normalFilter = oct_to_ndir_snorm(cb.inPrevDepthNormal[p].zw);

    //             if (is_reprojection_valid(prevPixel, depth.x, depthFilter.x, depth.y, normal, normalFilter, normalFWidth))
    //             {
    //                 prevIllumination += cb.inPrevIllumination.get()[p];
    //                 prevMoments += cb.inPrevMoments.get()[p].xy;
    //                 nValid += 1.0;
    //             }
    //         }
    //     }

    //     if (nValid > 0.0)
    //     {
    //         valid = true;
    //         prevIllumination /= nValid;
    //         prevMoments /= nValid;
    //     }
    // }

    if (valid)
    {
        historyLength = cb.inPrevHistoryLength.get()[prevPixel].x;
        // prevIllumination = cb.inPrevIllumination[prevPixel];
        // prevMoments = cb.inPrevMoments.get()[prevPixel].xy;
    }
    else
    {
        prevIllumination = 0;
        prevMoments = 0;
        historyLength = 0;
    }

    return valid;
}

void svgf_reproject(in float3 color, in float normalFWidth)
{
    const uint2 pixel = DispatchRaysIndex().xy;
    const float3 albedo = cb.outAlbedo[pixel].xyz;
    const float3 emission = 0;
    const float2 depth = cb.outDepthNormal[pixel].xy;
    const float3 normal = oct_to_ndir_snorm(cb.outDepthNormal[pixel].zw);

    float3 illumination = demodulate(color - emission, albedo);
    if (any(isnan(illumination)))
        illumination = 0;

    float historyLength;
    float4 prevIllumination;
    float2 prevMoments;
    bool success = load_previous_data(normalFWidth, normal, depth, prevIllumination, prevMoments, historyLength);
    historyLength = min(32, success ? historyLength + 1.0 : 1.0);

    const float alpha = success ? max(cb.alpha, 1.0 / historyLength) : 1.0;
    const float alphaMoments = success ? max(cb.momentsAlpha, 1.0 / historyLength) : 1.0;

    float2 moments;
    moments.r = luminance(illumination);
    moments.g = moments.r * moments.r;

    moments = lerp(prevMoments, moments, alphaMoments);

    float variance = max(0.0, moments.g - moments.r * moments.r);

    float4 finalIllumination = lerp(prevIllumination, float4(illumination, 0), alpha);
    finalIllumination.a = variance;

    cb.outIllumination.write(pixel, finalIllumination);
    cb.outMoments.write(pixel, moments);
    cb.outHistoryLength.write(pixel, historyLength);
    // cb.outColor.write(pixel, float4(illumination, 1));
}

#endif // SVGF_REPROJECTION