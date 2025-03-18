#ifndef COMMON_SHADER
#define COMMON_SHADER

#include "shader_interop_base.h"

#if defined(__spirv__)
   
static const uint BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER = 1;
static const uint BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER = 2;
static const uint BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER = 3;
static const uint BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE = 4;
static const uint BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE = 5;
static const uint BINDLESS_DESCRIPTOR_SET_SAMPLER = 6;

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] ByteAddressBuffer bindlessBuffers[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWByteAddressBuffer bindlessRWBuffers[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<uint> bindlessBuffersUint[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<uint2> bindlessBuffersUint2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<uint3> bindlessBuffersUint3[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<uint4> bindlessBuffersUint4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<float> bindlessBuffersFloat[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<float2> bindlessBuffersFloat2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<float3> bindlessBuffersFloat3[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<float4> bindlessBuffersFloat4[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<uint> bindlessRWBuffersUint[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<uint2> bindlessRWBuffersUint2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<uint3> bindlessRWBuffersUint3[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<uint4> bindlessRWBuffersUint4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<float> bindlessRWBuffersFloat[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<float2> bindlessRWBuffersFloat2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<float3> bindlessRWBuffersFloat3[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<float4> bindlessRWBuffersFloat4[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D bindlessTextures2D[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<float> bindlessTextures2DFloat[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<float2> bindlessTextures2DFloat2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<float4> bindlessTextures2DFloat4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<uint> bindlessTextures2DUint[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<uint2> bindlessTextures2DUint2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<uint4> bindlessTextures2DUint4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2DArray<float4> bindlessTexture2DArrays[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] TextureCube bindlessCubemaps[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] TextureCubeArray bindlessCubemapsArrays[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture3D bindlessTextures3D[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<float> bindlessRWTextures2DFloat[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<float4> bindlessRWTextures2DFloat4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<uint> bindlessRWTextures2DUint[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2DArray<float4> bindlessRWTexture2DArrays[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture3D<float4> bindlessRWTextures3D[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLER)]] SamplerState bindlessSamplers[];

#endif

#include "shader_interop_renderer.h"

#endif // COMMON_SHADER