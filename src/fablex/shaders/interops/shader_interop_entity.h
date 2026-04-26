#ifndef SHADER_INTEROP_ENTITY
#define SHADER_INTEROP_ENTITY

#include "shader_interop_base.h"

enum ShaderEntityType
{
	SHADER_ENTITY_TYPE_UNDEFINED = 0,
	SHADER_ENTITY_TYPE_DIRECTIONAL_LIGHT,
	SHADER_ENTITY_TYPE_POINT_LIGHT,
	SHADER_ENTITY_TYPE_SPOT_LIGHT,
};

// Must be aligned to 16 bytes. For now ShaderEntity supports point, spot and directional lights
struct ShaderEntity
{
	float3 position;
	uint type8Flags8;					// 16 bits are empty
	
	uint2 direction16OuterConeAngle16;	// There are 16 empty bits if RenderEntity type is dir light. I think it can be used in the future
	uint2 color;						// half4
	
	uint matrixIndex16TextureIndex16;
	uint attenuationRadius16;			// 16 bits are empty
	uint extraInfo16;					// Can contain two half floats. For spot light it will contain info about angles. For point light it will contain info about cubemap 
	uint empty;							// Will be used in the future

	void init()
	{
		position = float3(0, 0, 0);
		type8Flags8 = 0;
		direction16OuterConeAngle16 = uint2(0, 0);
		color = uint2(0, 0);
		matrixIndex16TextureIndex16 = 0;
		attenuationRadius16 = 0;
		extraInfo16 = 0;
		empty = 0;
	}

#ifndef __cplusplus
	inline uint get_type()
	{
		return type8Flags8 & 0xFF;
	}

	inline uint get_flags()
	{
		return (type8Flags8 >> 8) & 0xFF;
	}

	// Returns normalized direction
	inline float3 get_direction()
	{
		return normalize(float3(
			f16tof32(direction16OuterConeAngle16.x),
			f16tof32(direction16OuterConeAngle16.x >> 16u),
			f16tof32(direction16OuterConeAngle16.y)
		));
	}

	inline float get_outer_cone_angle()
	{
		return f16tof32(direction16OuterConeAngle16.y >> 16u);
	}

	inline float4 get_color()
	{
		return float4(
			f16tof32(color.x),
			f16tof32(color.x >> 16u),
			f16tof32(color.y),
			f16tof32(color.y >> 16u)
		);
	}

	inline uint get_matrix_index()
	{
		return matrixIndex16TextureIndex16 & 0xFFFF;
	}

	inline uint get_texture_index()
	{
		return matrixIndex16TextureIndex16 >> 16u;
	}

	inline float get_attenuation_radius()
	{
		return f16tof32(attenuationRadius16);
	}

	inline float get_angle_scale()
	{
		return f16tof32(extraInfo16);
	}

	inline float get_angle_offset()
	{
		return f16tof32(extraInfo16 >> 16u);
	}
#else
	inline void set_type(uint type)
	{
		type8Flags8 |= (type & 0xFF);
	}

	inline void set_flags(uint flags)
	{
		type8Flags8 |= (flags & 0xFF) << 8u;
	}

	inline void set_direction(float3 direction)
	{
		direction16OuterConeAngle16.x = DirectX::PackedVector::XMConvertFloatToHalf(direction.x);
		direction16OuterConeAngle16.x = DirectX::PackedVector::XMConvertFloatToHalf(direction.y) << 16u;
		direction16OuterConeAngle16.y = DirectX::PackedVector::XMConvertFloatToHalf(direction.z);
	}

	inline void set_outer_cone_angle_cos(float angle)
	{
		direction16OuterConeAngle16.y |= DirectX::PackedVector::XMConvertFloatToHalf(angle) << 16u;
	}

	inline void set_color(float4 colorValue)
	{
		color.x |= DirectX::PackedVector::XMConvertFloatToHalf(colorValue.x);
		color.x |= DirectX::PackedVector::XMConvertFloatToHalf(colorValue.y) << 16u;
		color.y |= DirectX::PackedVector::XMConvertFloatToHalf(colorValue.z);
		color.y |= DirectX::PackedVector::XMConvertFloatToHalf(colorValue.w) << 16u;
	}

	inline void set_indices(uint matrixIndex, uint textureIndex)
	{
		matrixIndex16TextureIndex16 |= (matrixIndex & 0xFFFF);
		matrixIndex16TextureIndex16 |= (textureIndex & 0xFFFF) << 16u;
	}

	inline void set_attenuation_radius(float radius)
	{
		attenuationRadius16 |= DirectX::PackedVector::XMConvertFloatToHalf(radius);
	}

	// For spot lights
	inline void set_angle_scale(float angleScale)
	{
		extraInfo16 |= DirectX::PackedVector::XMConvertFloatToHalf(angleScale);
	}

	// For spot lights
	inline void set_angle_offset(float angleOffset)
	{
		extraInfo16 |= DirectX::PackedVector::XMConvertFloatToHalf(angleOffset) << 16u;
	}
#endif	// __cplusplus
};

class ShaderEntityIterator
{
#ifndef __cplusplus
	uint offset;
	uint count;

	bool empty()
	{
		return count == 0;
	} 

	uint first_item()
	{
		return offset;
	}

	uint end_item()
	{
		return offset + count - 1;
	}

	uint random_item(inout RNG rng)
	{
		return offset + rng.next_uint(count);
	}
#endif // __cplusplus
};

#endif // SHADER_INTEROP_ENTITY