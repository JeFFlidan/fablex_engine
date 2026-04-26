#define OBJECT_SHADER_BASE_MODEL_LAYOUT
#include "object/object.hlsli"

struct PixelOutput
{
    float4 colorRGB_MetallicA : SV_Target0;
    float4 normalsRGB_RoughnessA : SV_Target1;
};

float4 main(PixelInput input) : SV_TARGET
{
    
}