#define OBJECT_SHADER_BASE_MODEL_LAYOUT
#include "object/object.hlsli"
#include "common/surface.hlsli"
#include "common/lighting.hlsli"

float4 main(PixelInput input) : SV_TARGET
{
	Surface surface;
	surface.P = input.position.xyz;
	surface.N = input.normal;

    FrameUB frame = get_frame();
    uint lightArrayOffset = frame.lightArrayOffset;
    uint lightArrayCount = frame.lightArrayCount;

    LightingResult lightingResult;

    for (uint i = lightArrayOffset; i != lightArrayOffset + lightArrayCount; ++i)
    {
        ShaderEntity entity = get_entity(i);
        switch (entity.get_type())
        {
        case SHADER_ENTITY_TYPE_POINT_LIGHT:
            break;
        case SHADER_ENTITY_TYPE_SPOT_LIGHT:
            break;
        case SHADER_ENTITY_TYPE_DIRECTIONAL_LIGHT:
            light_directional_lambert(entity, surface, lightingResult);
            break;
        }
    }

	return float4(lightingResult.direct.diffuse, 1.0);
}
