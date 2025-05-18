#include "common.hlsli"
#include "ray_tracing/svgf_common.hlsli"

PUSH_CONSTANTS(cb, SVGFFilterMomentsPushConstants)

[numthreads(SVGF_GROUP_SIZE, SVGF_GROUP_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int2 pixel = int2(DTid.xy);

    float h = cb.inHistoryLength.get()[pixel].x;
    float2 dimensions;
    cb.inHistoryLength.get().GetDimensions(dimensions.x, dimensions.y);

    if (h < 4.0)
    {
        float sumWIllumination = 0.0;
        float3 sumIllumination = 0.0;
        float2 sumMoments = 0.0;

        const float4 illuminationCenter = cb.inIllumination[pixel];
        const float lumIlluminationCenter = luminance(illuminationCenter.xyz);

        const float2 depthCenter = cb.inDepthNormal[pixel].xy;
        if (depthCenter.x <= 0)
        {
            cb.outFilteredIllumination.write(pixel, illuminationCenter);
            return;
        }

        const float3 normalCenter = oct_to_ndir_snorm(cb.inDepthNormal[pixel].zw);
        const float phiIllumination = cb.phiColor;
        const float phiDepth = max(depthCenter.y, 1e-8) * 3.0;

        const int radius = 3;
        for (int yy = -radius; yy <= radius; ++yy)
        {
            for (int xx = -radius; xx <= radius; ++xx)
            {
                const int2 p = pixel + int2(xx, yy);
                const bool inside = all(p >= 0) && all(p < int2(dimensions));

                if (inside)
                {
                    const float3 illuminationP = cb.inIllumination[p].xyz;
                    const float2 momentsP = cb.inMoments.get()[p].xy;
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
                        phiIllumination
                    );

                    sumWIllumination += w;
                    sumIllumination += illuminationP * w;
                    sumMoments += momentsP * w;
                }
            }
        }

        sumWIllumination = max(sumWIllumination, 1e-6f);
        sumIllumination /= sumWIllumination;
        sumMoments /= sumWIllumination;
        
        float variance = sumMoments.g - sumMoments.r * sumMoments.r;

        variance *= 4.0 / h;

        cb.outFilteredIllumination.write(pixel, float4(sumIllumination, variance));
    }
    else
    {
        cb.outFilteredIllumination.write(pixel, cb.inIllumination[pixel]);
    }
}