#ifndef SURFACE_HF
#define SURFACE_HF

#include "common.hlsli"

struct Surface
{
    float3 P;   // World space position
    float3 N;   // World space normal
    float3 V;   // World space view vec

    uint i0;
    uint i1;
    uint i2;
    float4 posWind0;
    float4 posWind1;
    float4 posWind2;

    float2 barycentrics;

    ShaderModelInstance instance;
    ShaderModel model;

    void init(in PrimitiveInfo primitiveInfo, in float2 inBarycentrics)
    {
        [branch]
        if (!load_model_data(primitiveInfo))
            return;

        barycentrics = inBarycentrics;

        P = barycentric_interpolation(
            posWind0.xyz,
            posWind1.xyz,
            posWind2.xyz,
            barycentrics
        );

        P = mul(instance.transform.get_matrix(), float4(P, 1.0)).xyz;

        init_internal();
    }

    bool load_model_data(in PrimitiveInfo primitiveInfo)
    {
        instance = get_model_instance(primitiveInfo.instanceIndex);
        model = get_model(instance.geometryOffset);

        uint3 indices = primitiveInfo.tri();
        i0 = indices.x;
        i1 = indices.y;
        i2 = indices.z;

        Buffer<float4> posBuffer = bindlessBuffersFloat4[descriptor_index(model.vertexBufferPosWind)];
        posWind0 = posBuffer[i0];
        posWind1 = posBuffer[i1];
        posWind2 = posBuffer[i2];

        return true;
    }

    void init_internal()
    {
        float3x3 adjointMat = instance.rawTransform.get_matrix_adjoint();
        
        [branch]
        if (model.vertexBufferNormals >= 0)
        {
            Buffer<float4> normalBuffer = bindlessBuffersFloat4[descriptor_index(model.vertexBufferNormals)];
            float3 n0 = mul(adjointMat, normalBuffer[i0].xyz);
            float3 n1 = mul(adjointMat, normalBuffer[i1].xyz);
            float3 n2 = mul(adjointMat, normalBuffer[i2].xyz);
            n0 = any(n0) ? normalize(n0) : 0;
            n1 = any(n1) ? normalize(n1) : 0;
            n2 = any(n2) ? normalize(n2) : 0;
            N = barycentric_interpolation(n0, n1, n2, barycentrics);
        }

        N = normalize(N);

        // TODO: UV
    }
};

#endif // SURFACE_HF