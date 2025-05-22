#include "common.hlsli"

PUSH_CONSTANTS(cb, ImGuiPushConstants);

struct VertexInput
{
    float2 position : POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;

    output.color = input.color;
    output.uv = input.uv;

    float2 scaled = input.position * cb.scale + cb.translate;
    output.position = float4(scaled, 0.0f, 1.0f);

    return output;
}
