#define OBJECT_SHADER_USE_COLOR
#include "object/object.hlsli"

float4 main(PixelInput input) : SV_TARGET
{
	return float4(input.color, 1.0f);
}
