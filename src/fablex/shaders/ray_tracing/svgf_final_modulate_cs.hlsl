#include "common.hlsli"

PUSH_CONSTANTS(cb, SVGFFinalModulatePushConstants)

[numthreads(SVGF_GROUP_SIZE, SVGF_GROUP_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 pixel = DTid.xy;
    // float4 color = cb.inAlbedo[pixel] * cb.inIllumination[pixel] + cb.inEmission[pixel];
    float4 color = float4(cb.inAlbedo[pixel].xyz, 1.0) * cb.inIllumination[pixel];

    cb.outColor.write(pixel, color);
}