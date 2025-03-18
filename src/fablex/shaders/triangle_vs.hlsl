struct VertInput
{
    uint vertexID : SV_VERTEXID;
};

struct PixInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

PixInput main(VertInput input)
{
    float3 positions[3] = {
        float3(-0.5f, -0.5f, 0.0f),
        float3(0.5f, -0.5f, 0.0f),
        float3(0.0f, 0.5f, 0.0f)
    };

    PixInput output;
    output.position = float4(positions[input.vertexID], 1.0f);
    output.color = float4(1.0f, 0.5f, 0.5f, 1.0f);
    return output;
}