#ifndef SHADER_INTEROP_DESCRIPTOR
#define SHADER_INTEROP_DESCRIPTOR

#include "shader_interop_base.h"

#ifdef __cplusplus
#define DEFINE_CPP_DESCRIPTOR()         \
    void operator=(uint inDescriptor)   \
    {                                   \
        descriptor = inDescriptor;      \
    }
#else
#define DEFINE_CPP_DESCRIPTOR()
#endif

#define DEFINE_TEXTURE_METHODS(BindlessArray, TextureType, ValueType, UVType)       \
    ValueType read(UVType uv)                                                       \
    {                                                                               \
        return BindlessArray[NonUniformResourceIndex(descriptor)][uv];              \
    }                                                                               \
    ValueType operator[](UVType uv)                                                 \
    {                                                                               \
        return BindlessArray[NonUniformResourceIndex(descriptor)][uv];              \
    }                                                                               \
    TextureType get()                                                               \
    {                                                                               \
        return BindlessArray[NonUniformResourceIndex(descriptor)];                  \
    }

#define DEFINE_RW_TEXTURE_METHODS(ClassName, BindlessArray, ValueType, UVType)      \
    template<> ValueType ClassName<ValueType>::read(UVType uv)                      \
    {                                                                               \
        return BindlessArray[NonUniformResourceIndex(descriptor)][uv];              \
    }                                                                               \
    template<> void ClassName<ValueType>::write(UVType uv, ValueType value)         \
    {                                                                               \
        BindlessArray[NonUniformResourceIndex(descriptor)][uv] = value;             \
    }                                                                               \
    template<> ValueType ClassName<ValueType>::operator[](UVType uv)                \
    {                                                                               \
        return BindlessArray[NonUniformResourceIndex(descriptor)][uv];              \
    }

#define DEFINE_RW_TEXTURE_2D_METHODS(BindlessArray, ValueType)          \
    DEFINE_RW_TEXTURE_METHODS(RWTexture2D_Descriptor, BindlessArray, ValueType, uint2);

#define DEFINE_RW_TEXTURE_2D_ARRAY_METHODS(BindlessArray, ValueType)    \
    DEFINE_RW_TEXTURE_METHODS(RWTexture2DArray_Descriptor, BindlessArray, ValueType, uint3);

#define DEFINE_RW_TEXTURE_3D_METHODS(BindlessArray, ValueType)          \
    DEFINE_RW_TEXTURE_METHODS(RWTexture3D_Descriptor, BindlessArray, ValueType, uint3);

struct Descriptor
{
    int descriptor;

    void init()
    {
        descriptor = -1;
    }

    bool is_valid()
    {
        return descriptor >= 0;
    }
};

template<typename T>
struct Texture2D_Descriptor : Descriptor
{
    DEFINE_CPP_DESCRIPTOR();

#ifndef __cplusplus
    DEFINE_TEXTURE_METHODS(bindlessTextures2D, Texture2D, T, uint2);
#endif // __cplusplus
};

template<typename T>
struct Texture2DArray_Descriptor : Descriptor
{
    DEFINE_CPP_DESCRIPTOR();

#ifndef __cplusplus
    DEFINE_TEXTURE_METHODS(bindlessTexture2DArrays, Texture2DArray, T, uint3);
#endif // __cplusplus
};

template<typename T>
struct TextureCube_Descriptor : Descriptor
{
    DEFINE_CPP_DESCRIPTOR();

#ifndef __cplusplus
    TextureCube get()
    {
        return bindlessCubemaps[NonUniformResourceIndex(descriptor)];
    }
#endif // __cplusplus
};

template<typename T>
struct TextureCubeArray_Descriptor : Descriptor
{
    DEFINE_CPP_DESCRIPTOR();

#ifndef __cplusplus
    TextureCubeArray get()
    {
        return bindlessCubemapsArrays[NonUniformResourceIndex(descriptor)];
    }
#endif
};

template<typename T>
struct Texture3D_Descriptor : Descriptor
{
    DEFINE_CPP_DESCRIPTOR();

#ifndef __cplusplus
    DEFINE_TEXTURE_METHODS(bindlessTextures3D, Texture3D, T, uint3);
#endif // __cplusplus
};

template<typename T>
struct RWTexture2D_Descriptor : Descriptor
{
    DEFINE_CPP_DESCRIPTOR();

#ifndef __cplusplus
    T read(uint2 uv) { return 0; }
    void write(uint2 uv, T value) { }
    T operator[](uint2 uv) { return 0; }
#endif // __cplusplus
};

template<typename T>
struct RWTexture2DArray_Descriptor : Descriptor
{
    DEFINE_CPP_DESCRIPTOR();

#ifndef __cplusplus
    T read(uint3 uv) { return 0; }
    void write(uint3 uv, T value) { }
    T operator[](uint3 uv) { return 0; }
#endif // __cplusplus
};

template<typename T>
struct RWTexture3D_Descriptor : Descriptor
{
    DEFINE_CPP_DESCRIPTOR();

#ifndef __cplusplus
    T read(uint3 uv) { return 0; }
    void write(uint3 uv, T value) { }
    T operator[](uint3 uv) { return 0; }
#endif // __cplusplus
};

#ifndef __cplusplus
DEFINE_RW_TEXTURE_2D_METHODS(bindlessRWTextures2DFloat, float);
DEFINE_RW_TEXTURE_2D_METHODS(bindlessRWTextures2DFloat2, float2);
DEFINE_RW_TEXTURE_2D_METHODS(bindlessRWTextures2DFloat4, float4);
DEFINE_RW_TEXTURE_2D_METHODS(bindlessRWTextures2DUInt, uint);
DEFINE_RW_TEXTURE_2D_METHODS(bindlessRWTextures2DUInt2, uint2);
DEFINE_RW_TEXTURE_2D_METHODS(bindlessRWTextures2DUInt4, uint4);
DEFINE_RW_TEXTURE_2D_METHODS(bindlessRWTextures2DInt, int);
DEFINE_RW_TEXTURE_2D_METHODS(bindlessRWTextures2DInt2, int2);
DEFINE_RW_TEXTURE_2D_METHODS(bindlessRWTextures2DInt4, int4);

DEFINE_RW_TEXTURE_2D_ARRAY_METHODS(bindlessRWTexture2DArraysFloat, float);
DEFINE_RW_TEXTURE_2D_ARRAY_METHODS(bindlessRWTexture2DArraysFloat2, float2);
DEFINE_RW_TEXTURE_2D_ARRAY_METHODS(bindlessRWTexture2DArraysFloat4, float4);
DEFINE_RW_TEXTURE_2D_ARRAY_METHODS(bindlessRWTexture2DArraysUInt, uint);
DEFINE_RW_TEXTURE_2D_ARRAY_METHODS(bindlessRWTexture2DArraysUInt2, uint2);
DEFINE_RW_TEXTURE_2D_ARRAY_METHODS(bindlessRWTexture2DArraysUInt4, uint4);
DEFINE_RW_TEXTURE_2D_ARRAY_METHODS(bindlessRWTexture2DArraysInt, int);
DEFINE_RW_TEXTURE_2D_ARRAY_METHODS(bindlessRWTexture2DArraysInt2, int2);
DEFINE_RW_TEXTURE_2D_ARRAY_METHODS(bindlessRWTexture2DArraysInt4, int4);

DEFINE_RW_TEXTURE_3D_METHODS(bindlessRWTextures3DFloat, float);
DEFINE_RW_TEXTURE_3D_METHODS(bindlessRWTextures3DFloat2, float2);
DEFINE_RW_TEXTURE_3D_METHODS(bindlessRWTextures3DFloat4, float4);
DEFINE_RW_TEXTURE_3D_METHODS(bindlessRWTextures3DUInt, uint);
DEFINE_RW_TEXTURE_3D_METHODS(bindlessRWTextures3DUInt2, uint2);
DEFINE_RW_TEXTURE_3D_METHODS(bindlessRWTextures3DUInt4, uint4);
DEFINE_RW_TEXTURE_3D_METHODS(bindlessRWTextures3DInt, int);
DEFINE_RW_TEXTURE_3D_METHODS(bindlessRWTextures3DInt2, int2);
DEFINE_RW_TEXTURE_3D_METHODS(bindlessRWTextures3DInt4, int4);

#endif // __cplusplus

struct AccelerationStructure_Descriptor : Descriptor
{
    DEFINE_CPP_DESCRIPTOR();

#ifndef __cplusplus
    RaytracingAccelerationStructure get()
    {
        return bindlessAccelerationStructures[NonUniformResourceIndex(descriptor)];
    }
#endif // __cplusplus
};

struct Sampler_Descriptor : Descriptor
{
    DEFINE_CPP_DESCRIPTOR();

#ifndef __cplusplus
    SamplerState get()
    {
        return bindlessSamplers[NonUniformResourceIndex(descriptor)];
    }
#endif // __cplusplus
};

#endif // SHADER_INTEROP_DESCRIPTOR