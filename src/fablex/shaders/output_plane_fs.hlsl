#include "common.hlsli"

PUSH_CONSTANTS(cb, SwapChainPushConstants)

struct PixelInput
{
	float4 position : SV_Position;
	float2 texCoords : TEXCOORD1;
};

float4 main(PixelInput input) : SV_TARGET
{
    SamplerState sampler = bindlessSamplers[SAMPLER_LINEAR_CLAMP];
	return cb.resultTexture.get().Sample(sampler, input.texCoords);
}
