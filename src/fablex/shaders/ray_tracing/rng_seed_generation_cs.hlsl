#include "common.hlsli"

PUSH_CONSTANTS(pushConstants, RNGSeedGenerationPushConstants)

[numthreads(RNG_SEED_GENERATION_GROUP_SIZE, RNG_SEED_GENERATION_GROUP_SIZE, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    uint2 pixelIndex = dtid.xy;
    RWTexture2D<uint4> rndSeedTexture = bindlessRWTextures2DUInt4[pushConstants.rngSeedTextureIndex];

    uint4 rngSeed = uint4(
        pixelIndex.x % pushConstants.blueNoiseTextureSize,
        pixelIndex.y % pushConstants.blueNoiseTextureSize,
        pushConstants.frameNumber % pushConstants.blueNoiseTextureDepth,
        false
    );

    rndSeedTexture[pixelIndex] = rngSeed;
}