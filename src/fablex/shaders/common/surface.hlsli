#ifndef SURFACE
#define SURFACE

#include "common.hlsli"

static const float MIN_ROUGHNESS = 0.035;

struct Surface
{
    float3 P;   // World space position
    float3 prevP;
    float3 N;   // World space normal
    float3 V;   // World space view vec

    uint i0;
    uint i1;
    uint i2;
    float4 posWind0;
    float4 posWind1;
    float4 posWind2;

    float4 baseColor;
    float roughness;
    float metallic;
    float occlusion;
    float3 F0;  // Fresnel value (specular color)
    float3 F;

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

        prevP = mul(instance.prevTransform.get_matrix(), float4(P, 1.0)).xyz;
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

        // TODO: Get values from textures
        ShaderMaterial shaderMaterial = get_material(instance.materialIndex);
        baseColor = shaderMaterial.get_base_color();
        roughness = shaderMaterial.get_roughness();
        roughness *= roughness;
        metallic = shaderMaterial.get_metallic();
        
        F0 = lerp(float3(0.04f, 0.04f, 0.04f), baseColor.xyz, metallic);

        // TODO: UV
    }

    float2 get_motion_vector(in ShaderCamera camera)
    {
        float4 currClip = mul(camera.viewProjection, float4(P, 1));
        float2 currUV = currClip.xy / currClip.w * 0.5 + 0.5;

        float4 prevClip = mul(camera.prevViewProjection, float4(prevP, 1));
        float2 prevUV = prevClip.xy / prevClip.w * 0.5 + 0.5;

        return currUV - prevUV;
    }
};

#endif // SURFACE