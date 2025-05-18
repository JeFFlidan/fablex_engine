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
    float2 barycentricsQuadX; // world space
    float2 barycentricsQuadY; // world space
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

#ifndef SURFACE_NORMAL_DERIVATIVES
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
#endif // SURFACE_NORMAL_DERIVATIVES

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

        P = barycentric_interpolation(P0.xyz, P1.xyz, P2.xyz, barycentrics);

#ifdef SURFACE_BARYCENTRICS_QUADS
        float3 rayDirectionX = QuadReadAcrossX(rayDirection);
        float3 rayDirectionY = QuadReadAcrossY(rayDirection);
        P0 = mul(instance.transform.get_matrix(), float4(P0, 1)).xyz;
        P1 = mul(instance.transform.get_matrix(), float4(P1, 1)).xyz;
        P2 = mul(instance.transform.get_matrix(), float4(P2, 1)).xyz;
        barycentricsQuadX = compute_barycentrics(rayOrigin, rayDirectionX, P0.xyz, P1.xyz, P2.xyz);
        barycentricsQuadY = compute_barycentrics(rayOrigin, rayDirectionY, P0.xyz, P1.xyz, P2.xyz);
#endif

#ifdef SURFACE_NORMAL_DERIVATIVES
        init_internal(rayOrigin, rayDirectionX, rayDirectionY);
#else
        init_internal();
#endif // SURFACE_NORMAL_DERIVATIVES


#ifdef SURFACE_BARYCENTRICS_QUADS
#endif // SURFACE_BARYCENTRICS_QUADS

#ifdef SURFACE_POSITION_DERIVATIVES
        P_quadX = barycentric_interpolation(P0.xyz, P1.xyz, P2.xyz, barycentricsQuadX);
        P_quadY = barycentric_interpolation(P0.xyz, P1.xyz, P2.xyz, barycentricsQuadY);
        P_dx = P - P_quadX;
        P_dy = P - P_quadY;

        // P_quadX = mul(instance.transform.get_matrix(), float4(P_quadX, 1)).xyz;
        // P_quadY = mul(instance.transform.get_matrix(), float4(P_quadY, 1)).xyz;
#endif
    }

    bool load_model_data(in PrimitiveInfo primitiveInfo)
    {
        instance = get_model_instance(primitiveInfo.instanceIndex);
        model = get_model(instance.geometryOffset);

        uint3 indices = primitiveInfo.tri();
        i0 = indices.x;
        i1 = indices.y;
        i2 = indices.z;

        get_tri(P0, P1, P2);

        return true;
    }

    void init_internal(
#ifdef SURFACE_NORMAL_DERIVATIVES
        float3 rayOrigin,
        float3 rayDirectionX,
        float3 rayDirectionY
#endif
    )
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
            // n0 = normalBuffer[i0].xyz;
            // n1 = normalBuffer[i1].xyz;
            // n2 = normalBuffer[i2].xyz;

            // float3 localP0, localP1, localP2;
            // get_tri(localP0, localP1, localP2);

            // float2 normalBarycentricsQuadX = compute_barycentrics(rayOrigin, rayDirectionX, localP0, localP1, localP2);
            // float2 normalBarycentricsQuadY = compute_barycentrics(rayOrigin, rayDirectionY, localP0, localP1, localP2);

            N_dx = barycentric_interpolation(n0, n1, n2, barycentricsQuadX);
            N_dy = barycentric_interpolation(n0, n1, n2, barycentricsQuadY);
            
            N_dx = normalize(N_dx);
            N_dy = normalize(N_dy);

            // N_dx = mul(adjointMat, N_dx);
            // N_dy = mul(adjointMat, N_dy);

            N_dx = N - N_dx;
            N_dy = N - N_dy;

            // N_dy = 0;
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

    void get_tri(out float3 localP0, out float3 localP1, out float3 localP2)
    {
        Buffer<float4> posBuffer = bindlessBuffersFloat4[descriptor_index(model.vertexBufferPosWind)];
        localP0 = posBuffer[i0].xyz;
        localP1 = posBuffer[i1].xyz;
        localP2 = posBuffer[i2].xyz;
    }
};

#endif // SURFACE