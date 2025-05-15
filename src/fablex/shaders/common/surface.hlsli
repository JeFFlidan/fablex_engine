#ifndef SURFACE
#define SURFACE

#if defined(SURFACE_NORMAL_DERIVATIVES) || defined(SURFACE_POSITION_DERIVATIVES)
#define SURFACE_BARYCENTRICS_QUADS
#endif

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
    float3 P0;
    float3 P1;
    float3 P2;

    float4 baseColor;
    float3 emissionColor;
    float roughness;
    float metallic;
    float occlusion;
    float3 F0;  // Fresnel value (specular color)
    float3 F;

    float2 barycentrics;

#ifdef SURFACE_BARYCENTRICS_QUADS
    float2 barycentricsQuadX;
    float2 barycentricsQuadY;
#endif // SURFACE_BARYCENTRICS_QUADS

#ifdef SURFACE_NORMAL_DERIVATIVES
    float3 N_dx;
    float3 N_dy;
#endif // SURFACE_NORMAL_DERIVATIVES

#ifdef SURFACE_POSITION_DERIVATIVES
    float3 P_quadX;
    float3 P_quadY;
    float3 P_dx;
    float3 P_dy;
#endif // SURFACE_POSITION_DERIVATIVES

    ShaderModelInstance instance;
    ShaderModel model;

    void init(in PrimitiveInfo primitiveInfo, in float2 inBarycentrics)
    {
        [branch]
        if (!load_model_data(primitiveInfo))
            return;

        barycentrics = inBarycentrics;

        P = barycentric_interpolation(
            P0.xyz,
            P1.xyz,
            P2.xyz,
            barycentrics
        );

        init_internal();
    }

    void init(
        in PrimitiveInfo primitiveInfo, 
        in float2 inBarycentrics,
        in float3 rayOrigin,
        in float3 rayDirection
    )
    {
        [branch]
        if (!load_model_data(primitiveInfo))
            return;

        barycentrics = inBarycentrics;

#ifdef SURFACE_BARYCENTRICS_QUADS
        barycentricsQuadX = compute_barycentrics(rayOrigin, QuadReadAcrossX(rayDirection), P0.xyz, P1.xyz, P2.xyz);
        barycentricsQuadY = compute_barycentrics(rayOrigin, QuadReadAcrossY(rayDirection), P0.xyz, P1.xyz, P2.xyz);
#endif // SURFACE_BARYCENTRICS_QUADS

        P = barycentric_interpolation(P0.xyz, P1.xyz, P2.xyz, barycentrics);

#ifdef SURFACE_POSITION_DERIVATIVES
        P_quadX = barycentric_interpolation(P0.xyz, P1.xyz, P2.xyz, barycentricsQuadX);
        P_quadY = barycentric_interpolation(P0.xyz, P1.xyz, P2.xyz, barycentricsQuadY);
        P_dx = P - P_quadX;
        P_dy = P - P_quadY;

        P_quadX = mul(instance.transform.get_matrix(), float4(P_quadX, 1)).xyz;
        P_quadY = mul(instance.transform.get_matrix(), float4(P_quadY, 1)).xyz;
#endif

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
        P0 = posBuffer[i0].xyz;
        P1 = posBuffer[i1].xyz;
        P2 = posBuffer[i2].xyz;

        return true;
    }

    void init_internal()
    {
        prevP = mul(instance.prevTransform.get_matrix(), float4(P, 1.0)).xyz;
        P = mul(instance.transform.get_matrix(), float4(P, 1.0)).xyz;

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

            N = normalize(N);

#ifdef SURFACE_NORMAL_DERIVATIVES
            N_dx = barycentric_interpolation(n0, n1, n2, barycentricsQuadX);
            N_dy = barycentric_interpolation(n0, n1, n2, barycentricsQuadY);

            N_dx = normalize(N_dx);
            N_dy = normalize(N_dy);

            N_dx = N - N_dx;
            N_dy = N - N_dy;
#endif
        }

        // TODO: Get values from textures
        ShaderMaterial shaderMaterial = get_material(instance.materialIndex);
        baseColor = shaderMaterial.get_base_color();
        roughness = shaderMaterial.get_roughness();
        roughness *= roughness;
        metallic = shaderMaterial.get_metallic();
        emissionColor = 0;   // TEMP
        
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