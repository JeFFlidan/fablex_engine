struct VertexInput
{
	uint vertexID : SV_VertexID;
};

struct PixelInput
{
	float4 position : SV_Position;
	float2 texCoords : TEXCOORD1;
};

PixelInput main(VertexInput input)
{
    float3 positions[6] = {
        float3(-1.0f, 1.0f, 0.0f),  // Top-left corner
        float3(-1.0f, -1.0f, 0.0f), // Bottom-left corner
        float3(1.0f, -1.0f, 0.0f),  // Bottom-right corner
        float3(1.0f, -1.0f, 0.0f),  // Bottom-right corner
        float3(1.0f, 1.0f, 0.0f),   // Top-right corner
        float3(-1.0f, 1.0f, 0.0f)   // Top-left corner
    };

    float2 texCoords[6] = {
        float2(0.0f, 0.0f),  // Top-left corner
        float2(0.0f, 1.0f),  // Bottom-left corner
        float2(1.0f, 1.0f),  // Bottom-right corner
        float2(1.0f, 1.0f),  // Bottom-right corner
        float2(1.0f, 0.0f),  // Top-right corner
        float2(0.0f, 0.0f)   // Top-left corner
    };

	PixelInput output;
	output.position = float4(positions[input.vertexID], 1.0f);
	output.texCoords = texCoords[input.vertexID];
	return output;
}
