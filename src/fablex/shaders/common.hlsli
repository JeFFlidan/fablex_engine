#ifndef COMMON_SHADER
#define COMMON_SHADER

#include "shader_interop_base.h"
#include "utils/rng.hlsli"
#include "utils/math.hlsli"

uint descriptor_index(uint rawIndex)
{
    return NonUniformResourceIndex(rawIndex);
}

#if defined(__spirv__)

static const uint BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER = 1;
static const uint BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER = 2;
static const uint BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER = 3;
static const uint BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE = 4;
static const uint BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE = 5;
static const uint BINDLESS_DESCRIPTOR_SET_SAMPLER = 6;
static const uint BINDLESS_DESCRIPTOR_SET_ACCELERATION_STRUCTURE = 7;

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] ByteAddressBuffer bindlessBuffers[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWByteAddressBuffer bindlessRWBuffers[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<uint> bindlessBuffersUInt[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<uint2> bindlessBuffersUInt2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<uint3> bindlessBuffersUInt3[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<uint4> bindlessBuffersUInt4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<float> bindlessBuffersFloat[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<float2> bindlessBuffersFloat2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<float3> bindlessBuffersFloat3[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_UNIFORM_TEXEL_BUFFER)]] Buffer<float4> bindlessBuffersFloat4[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<uint> bindlessRWBuffersUInt[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<uint2> bindlessRWBuffersUInt2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<uint3> bindlessRWBuffersUInt3[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<uint4> bindlessRWBuffersUInt4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<float> bindlessRWBuffersFloat[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<float2> bindlessRWBuffersFloat2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<float3> bindlessRWBuffersFloat3[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_TEXEL_BUFFER)]] RWBuffer<float4> bindlessRWBuffersFloat4[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D bindlessTextures2D[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<float> bindlessTextures2DFloat[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<float2> bindlessTextures2DFloat2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<float4> bindlessTextures2DFloat4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<uint> bindlessTextures2DUInt[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<uint2> bindlessTextures2DUInt2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2D<uint4> bindlessTextures2DUInt4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture2DArray bindlessTexture2DArrays[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] TextureCube bindlessCubemaps[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] TextureCubeArray bindlessCubemapsArrays[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] Texture3D bindlessTextures3D[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<float> bindlessRWTextures2DFloat[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<float2> bindlessRWTextures2DFloat2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<float4> bindlessRWTextures2DFloat4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<uint> bindlessRWTextures2DUInt[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<uint2> bindlessRWTextures2DUInt2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<uint4> bindlessRWTextures2DUInt4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<int> bindlessRWTextures2DInt[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<int2> bindlessRWTextures2DInt2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2D<int4> bindlessRWTextures2DInt4[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2DArray<float> bindlessRWTexture2DArraysFloat[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2DArray<float2> bindlessRWTexture2DArraysFloat2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture2DArray<float4> bindlessRWTexture2DArraysFloat4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] RWTexture2DArray<uint> bindlessRWTexture2DArraysUInt[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] RWTexture2DArray<uint2> bindlessRWTexture2DArraysUInt2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] RWTexture2DArray<uint4> bindlessRWTexture2DArraysUInt4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] RWTexture2DArray<int> bindlessRWTexture2DArraysInt[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] RWTexture2DArray<int2> bindlessRWTexture2DArraysInt2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLED_IMAGE)]] RWTexture2DArray<int4> bindlessRWTexture2DArraysInt4[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture3D<float> bindlessRWTextures3DFloat[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture3D<float2> bindlessRWTextures3DFloat2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture3D<float4> bindlessRWTextures3DFloat4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture3D<uint> bindlessRWTextures3DUInt[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture3D<uint2> bindlessRWTextures3DUInt2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture3D<uint4> bindlessRWTextures3DUInt4[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture3D<int> bindlessRWTextures3DInt[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture3D<int2> bindlessRWTextures3DInt2[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_IMAGE)]] RWTexture3D<int4> bindlessRWTextures3DInt4[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_SAMPLER)]] SamplerState bindlessSamplers[];

[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_ACCELERATION_STRUCTURE)]] RaytracingAccelerationStructure bindlessAccelerationStructures[];

#endif

#include "shader_interop_renderer.h"

#if defined(__spirv__)
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWStructuredBuffer<ShaderModel> bindlessStructuredModels[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWStructuredBuffer<ShaderModelInstance> bindlessStructuredModelInstances[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWStructuredBuffer<ShaderModelInstanceID> bindlessStructuredModelInstanceIDs[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWStructuredBuffer<ShaderMaterial> bindlessStructuredMaterials[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWStructuredBuffer<ShaderEntity> bindlessStructuredEntities[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWStructuredBuffer<ShaderMeshlet> bindlesStructuredMeshlets[];
[[vk::binding(0, BINDLESS_DESCRIPTOR_SET_STORAGE_BUFFER)]] RWStructuredBuffer<ShaderMeshletBounds> bindlesStructuredMeshletBounds[];
#endif

FrameUB get_frame()
{
    return frameData;
}

inline ShaderCamera get_camera(uint cameraIndex = 0)
{
    if (cameraIndex < MAX_CAMERA_COUNT)
        return cameraData.cameras[cameraIndex];
    return cameraData.cameras[0];
}

inline ShaderEntity get_entity(uint entityIndex)
{
    return bindlessStructuredEntities[get_frame().entityBufferIndex][entityIndex];
}

inline ShaderMaterial get_material(uint materialIndex)
{
    return bindlessStructuredMaterials[get_frame().materialBufferIndex][materialIndex];
}

inline ShaderModel get_model(uint modelIndex)
{
    return bindlessStructuredModels[get_frame().modelBufferIndex][modelIndex];
}

inline ShaderModelInstanceID get_model_instance_id(uint hlslInstanceID)
{
    return bindlessStructuredModelInstanceIDs[get_frame().modelInstanceIDBufferIndex][hlslInstanceID];
}

inline ShaderModelInstance get_model_instance(uint hlslInstanceID)
{
    // TEMP!!!
    return bindlessStructuredModelInstances[get_frame().modelInstanceBufferIndex][hlslInstanceID];
}

inline ShaderEntityIterator lights_iter()
{
    FrameUB frame = get_frame();
    ShaderEntityIterator iterator;
    iterator.count = frame.lightArrayCount;
    iterator.offset = frame.lightArrayOffset;
    return iterator;
}

struct PrimitiveInfo
{
    uint primitiveIndex;
    uint instanceIndex;
    uint meshIndex;
    uint padding;

    void init()
    {
        primitiveIndex = 0;
        instanceIndex = 0;
        meshIndex = 0;
        padding = 0;
    }

    uint3 tri()
    {
        ShaderModelInstance instance = get_model_instance(instanceIndex);
        ShaderModel model = get_model(instance.geometryOffset + meshIndex); // TEMP, THINK HOW TO FIND GEOMETRY INDEX
        
        const uint beginIndex = primitiveIndex * 3 + model.indexOffset;
        Buffer<uint> indexBuffer = bindlessBuffersUInt[descriptor_index(model.indexBuffer)];

        return uint3(
            indexBuffer[beginIndex],
            indexBuffer[beginIndex + 1],
            indexBuffer[beginIndex + 2]
        );
    }
};

#endif // COMMON_SHADER