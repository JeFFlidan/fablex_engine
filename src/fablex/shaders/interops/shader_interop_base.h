#ifndef SHADER_INTEROP
#define SHADER_INTEROP

#ifdef __cplusplus
#include "core/types.h"
#include "core/math.h"

using float2 = fe::Float2;
using float3 = fe::Float3;
using float4 = fe::Float4;
using uint = uint32;
using uint2 = fe::UInt2;
using uint3 = fe::UInt3;
using uint4 = fe::UInt4;
using int2 = fe::Int2;
using int3 = fe::Int3;
using int4 = fe::Int4;
using float4x4 = fe::Float4x4;

#define UNIFORM_BUFFER(Name, Type, Slot)
#define PUSH_CONSTANTS(Name, Type)

#else

#define alignas(x)

#define PASTE1(a, b) a##b
#define PASTE(a, b) PASTE1(a, b)
#define UNIFORM_BUFFER(Name, Type, Slot) ConstantBuffer<Type> Name : register(PASTE(b, Slot))

#if defined(__spirv__)
#define PUSH_CONSTANTS(Name, Type) [[vk::push_constant]] Type Name;
#endif

#endif // __cplusplus

#define UB_FRAME_SLOT		0
#define UB_CAMERA_SLOT		1

struct ShaderSphereBounds
{
	float3 center;
	float radius;

	void init()
	{
		center = float3(0.0f, 0.0f, 0.0f);
		radius = 0.0f;
	}
};

struct ShaderFrustum
{
	float4 planes[6];
#ifndef __cplusplus
	bool check(ShaderSphereBounds sphereBounds)
	{
		bool visible = true;
		[unroll]
		for (int i = 0; i != 6; ++i)
		{
			visible = visible && dot(planes[i], float4(sphereBounds.center, 1.0f)) > -sphereBounds.radius;
		}
		return visible;
	}
#endif
};

struct Viewport
{
	float left;
	float top;
	float right;
	float bottom;
};

#endif // SHADER_INTEROP