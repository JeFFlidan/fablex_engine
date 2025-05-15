#include "common.hlsli"

PUSH_CONSTANTS(cb, SVGFFinalModulatePushConstants)

[numthreads(SVGF_GROUP_SIZE, SVGF_GROUP_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 pixel = DTid.xy;
    // TODO: Main logic
    cb.outColor.write(pixel, cb.inIllumination[pixel]);
}