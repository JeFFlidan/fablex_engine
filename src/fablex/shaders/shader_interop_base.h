#ifndef SHADER_INTEROP
#define SHADER_INTEROP

#ifdef __cplusplus
#include "core/types.h"
#include "core/math.h"

using float2 = fe::math::Float2;
using float3 = fe::math::Float3;
using float4 = fe::math::Float4;
using uint = uint32;
using uint2 = fe::math::UInt2;
using uint3 = fe::math::UInt3;
using uint4 = fe::math::UInt4;
using int2 = fe::math::Int2;
using int3 = fe::math::Int3;
using int4 = fe::math::Int4;
using float4x4 = fe::math::Float4x4;

#define UNIFORM_BUFFER(Name, Type, Slot)
#define PUSH_CONSTANTS(Name, Type)

#else

#define PASTE1(a, b) a##b
#define PASTE(a, b) PASTE1(a, b)
#define UNIFORM_BUFFER(Name, Type, Slot) ConstantBuffer<Type> Name : register(PASTE(b, Slot))

#if defined(__spirv__)
#define PUSH_CONSTANTS(Name, Type) [[vk::push_constant]] Type Name;
#endif

#endif // __cplusplus

#define UB_FRAME_SLOT		0
#define UB_CAMERA_SLOT		1

#endif // SHADER_INTEROP