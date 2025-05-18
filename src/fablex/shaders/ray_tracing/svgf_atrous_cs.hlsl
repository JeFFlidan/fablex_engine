#include "common.hlsli"
#include "ray_tracing/svgf_common.hlsli"

PUSH_CONSTANTS(cb, SVGFAtrousPushConstants);

float compute_variance_center(int2 pixel)
{
    float sum = 0.0f;

    const float kernel[2][2] = {
        {1.0 / 4.0, 1.0 / 8.0},
        {1.0 / 8.0, 1.0 / 16.0}
    };

    const int radius = 1;
    for (int yy = -radius; yy <= radius; ++yy)
    {
        for (int xx = -radius; xx <= radius; ++xx)
        {
            const int2 p = pixel + int2(xx, yy);
            const float k = kernel[abs(xx)][abs(yy)];
            sum += cb.inIllumination[p].a * k;
        }
    }

    return sum;
}

[numthreads(SVGF_GROUP_SIZE, SVGF_GROUP_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int2 pixel = int2(DTid.xy);
    
    float2 dimensions;
    cb.inHistoryLength.get().GetDimensions(dimensions.x, dimensions.y);

    const float epsVariance = 1e-10;
    const float kernelWeights[3] = {1.0, 2.0 / 3.0, 1.0 / 6.0};

    const float4 illuminationCenter = cb.inIllumination.read(pixel);
    const float lumIlluminationCenter = luminance(illuminationCenter.xyz);

    const float var = compute_variance_center(pixel);

    const float historyLength = cb.inHistoryLength.get()[pixel].x;

    const float2 depthCenter = cb.inDepthNormal.read(pixel).xy;
    if (depthCenter.x <= 0)
    {
        cb.outFinalIllumination.write(pixel, illuminationCenter);
        return;
    }

    const float3 normalCenter = oct_to_ndir_snorm(cb.inDepthNormal[pixel].zw);

    const float phiLumIllumination = cb.phiColor * sqrt(max(0.0, epsVariance + var.r));
    const float phiDepth = max(depthCenter.y, 1e-8) * cb.stepSize;

    float sumWIllumination = 1.0;
    float4 sumIllumination = illuminationCenter;

    int radius = 2;
    for (int yy = -radius; yy <= radius; ++yy)
    {
        for (int xx = -radius; xx <= radius; ++xx)
        {
            const int2 p = pixel + int2(xx, yy);
            const bool inside = all(p >= 0) && all(p < int2(dimensions));

            const float kernel = kernelWeights[abs(xx)] * kernelWeights[abs(yy)];

            if (inside && (xx != 0 || yy != 0))
            {
                const float4 illuminationP = cb.inIllumination[p];
                const float lumIlluminationP = luminance(illuminationP.xyz);
                const float depthP = cb.inDepthNormal[p].x;
                const float3 normalP = oct_to_ndir_snorm(cb.inDepthNormal[p].zw);

                const float w = compute_weight(
                    depthCenter.x,
                    depthP,
                    phiDepth * length(float2(xx, yy)),
                    normalCenter,
                    normalP,
                    cb.phiNormal,
                    lumIlluminationCenter,
                    lumIlluminationP,
                    phiLumIllumination
                );

                const float wIllumination = w * kernel;

                sumWIllumination += wIllumination;
                sumIllumination += float4(wIllumination.xxx, wIllumination * wIllumination) * illuminationP;
            }
        }
    }

    float4 filteredIllumination = float4(sumIllumination / float4(sumWIllumination.xxx, sumWIllumination * sumWIllumination));

    // TODO: Main logic
    cb.outFinalIllumination.write(pixel, filteredIllumination);
}