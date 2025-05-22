#include "common.hlsli"

PUSH_CONSTANTS(cb, ImGuiPushConstants);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv    : TEXCOORD0;
    float4 color : COLOR0;
};

float4 main(PSInput input) : SV_Target
{
    float4 textureColor = cb.texture.get().Sample(cb.sampler.get(), input.uv);
    return input.color * textureColor;
}