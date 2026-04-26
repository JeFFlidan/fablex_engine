#ifndef SHADER_INTEROP_MATERIAL
#define SHADER_INTEROP_MATERIAL

#include "shader_interop_base.h"
#include "shader_interop_descriptor.h"

static const uint SHADER_MATERIAL_FLAG_TRANSPARENT = 1 << 0;
static const uint SHADER_MATERIAL_FLAG_UNLIT = 1 << 1;

enum TextureSlot
{
	TEXTURE_SLOT_BASE_COLOR = 0,
	TEXTURE_SLOT_NORMAL,
	TEXTURE_SLOT_ROUGHNESS,
	TEXTURE_SLOT_METALLIC,
	TEXTURE_SLOT_AO,
	TEXTURE_SLOT_ARM,	// AO, roughness, metallic

	TEXTURE_SLOT_COUNT
};

enum SamplerType
{
	SAMPLER_LINEAR_REPEAT = 0,
	SAMPLER_LINEAR_CLAMP,
	SAMPLER_LINEAR_MIRROR,

	SAMPLER_NEAREST_REPEAT,
	SAMPLER_NEAREST_CLAMP,
	SAMPLER_NEAREST_MIRROR,

	SAMPLER_MINIMUM_NEAREST_CLAMP,

	SAMPLER_COUNT
};

struct ShaderTexture2D
{
	int textureIndex;
	float3 empty;

	inline void init()
	{
		textureIndex = -1;
	}

	inline bool is_valid()
	{
		return textureIndex > 0;
	}

#ifndef __cplusplus
	Texture2D get_texture()
	{
		return bindlessTextures2DFloat4[NonUniformResourceIndex(textureIndex)];
	}

	float4 sample(in SamplerState sampler, in float4 uv_sets)
	{
		Texture2D texture = get_texture();
		float2 uv = uv_sets.xy;		// Have to think how to work with 2 uv sets
		return texture.Sample(sampler, uv);
	}
#endif
};

struct ShaderMaterial
{
	uint2 baseColor;	//half4
	uint roughness16Metallic16;
	uint opacity16SamplerIndex16;
	uint flags8;		// 24 bits are empty
	float3 empty;

	Texture2D_Descriptor<float4> textures[TEXTURE_SLOT_COUNT];

#ifndef __cplusplus
	inline float4 get_base_color()
	{
		return float4(
			f16tof32(baseColor.x),
			f16tof32(baseColor.x >> 16u),
			f16tof32(baseColor.y),
			f16tof32(baseColor.y >> 16u)
		);
	}

	inline float get_roughness()
	{
		return f16tof32(roughness16Metallic16);
	}

	inline float get_metallic()
	{
		return f16tof32(roughness16Metallic16 >> 16u);
	}

	inline uint get_sampler_index()
	{
		return opacity16SamplerIndex16 >> 16u;
	}

	void get_sampler(out SamplerState sampler)
	{
		uint samplerIndex = opacity16SamplerIndex16 >> 16u;
		sampler = bindlessSamplers[NonUniformResourceIndex(samplerIndex)];
	}

	inline float get_opacity()
	{
		return f16tof32(opacity16SamplerIndex16);
	}

	inline float4 sample(TextureSlot textureSlot, in float4 uvSets)
	{
		SamplerState sampler = bindlessSamplers[descriptor_index(get_sampler_index())];
		return textures[textureSlot].get().Sample(sampler, uvSets.xy);
	}

	inline float4 sample(TextureSlot textureSlot, in float4 uvSets, in float lod)
	{
		SamplerState sampler = bindlessSamplers[descriptor_index(get_sampler_index())];
		return textures[textureSlot].get().SampleLevel(sampler, uvSets.xy, lod);
	}

#else
	void init()
	{
		baseColor = uint2(0, 0);
		roughness16Metallic16 = 0;
		opacity16SamplerIndex16 = 0;
		flags8 = 0;
	}

	inline void set_base_color(float4 inBaseColor)
	{
		baseColor.x |= DirectX::PackedVector::XMConvertFloatToHalf(inBaseColor.x);
		baseColor.x |= DirectX::PackedVector::XMConvertFloatToHalf(inBaseColor.y) << 16u;
		baseColor.y |= DirectX::PackedVector::XMConvertFloatToHalf(inBaseColor.z);
		baseColor.y |= DirectX::PackedVector::XMConvertFloatToHalf(inBaseColor.w) << 16u;
	}

	inline void set_roughness(float roughness)
	{
		roughness16Metallic16 |= DirectX::PackedVector::XMConvertFloatToHalf(roughness);
	}

	inline void set_metallic(float metallic)
	{
		roughness16Metallic16 |= DirectX::PackedVector::XMConvertFloatToHalf(metallic) << 16u;
	}

	inline void set_opacity(float opacity)
	{
		opacity16SamplerIndex16 |= DirectX::PackedVector::XMConvertFloatToHalf(opacity);
	}

	inline void set_sampler_index(uint samplerIndex)
	{
		opacity16SamplerIndex16 |= samplerIndex << 16u;
	}

	inline void set_flags(uint flags)
	{
		flags8 |= (flags & 0xFF);
	}
#endif

	inline bool is_transparent()
	{
		uint flags = flags8 & 0xFF;
		return flags & SHADER_MATERIAL_FLAG_TRANSPARENT;
	}

	inline bool is_unlit()
	{
		uint flags = flags8 & 0xFF;
		return flags & SHADER_MATERIAL_FLAG_UNLIT;
	}
};

#endif // SHADER_INTEROP_MATERIAL