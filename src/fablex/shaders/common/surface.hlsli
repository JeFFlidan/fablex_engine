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
    float4 T;   // Tangent
    float3 V;   // World space view vec

    uint i0;
    uint i1;
    uint i2;
    float3 P0;
    float3 P1;
    float3 P2;

    float4 baseColor;
    float4 emissionColor;
    float3 bumpColor;
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

        P0 = mul(instance.transform.get_matrix(), float4(P0, 1)).xyz;
        P1 = mul(instance.transform.get_matrix(), float4(P1, 1)).xyz;
        P2 = mul(instance.transform.get_matrix(), float4(P2, 1)).xyz;

#ifdef SURFACE_BARYCENTRICS_QUADS
        float3 rayDirectionX = QuadReadAcrossX(rayDirection);
        float3 rayDirectionY = QuadReadAcrossY(rayDirection);
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
#endif
    }

    bool load_model_data(in PrimitiveInfo primitiveInfo)
    {
        P = 0;
        prevP = 0;
        N = 0;
        T = 0;
        baseColor = 0;
        emissionColor = 0;
        bumpColor = 0;
        roughness = 0;
        metallic = 0;
        occlusion = 1;
        F0 = 0;
        F = 0;
        barycentrics = 0;        

        instance = get_model_instance(primitiveInfo.instanceIndex);
        model = get_model(instance.meshOffset);

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

        ShaderMaterial material = get_material(instance.materialIndex);

        float3 unnormalizedN = 0;
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

            unnormalizedN = N;

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

        float4 uvSets;

        [branch]
        if (model.vertexBufferUVs >= 0)
        {
            Buffer<float4> uvsBuffer = bindlessBuffersFloat4[descriptor_index(model.vertexBufferUVs)];
            float4 uv0 = lerp(model.uvRangeMin.xyxy, model.uvRangeMax.xyxy, uvsBuffer[i0]);
            float4 uv1 = lerp(model.uvRangeMin.xyxy, model.uvRangeMax.xyxy, uvsBuffer[i1]);
            float4 uv2 = lerp(model.uvRangeMin.xyxy, model.uvRangeMax.xyxy, uvsBuffer[i2]);
            // TODO: Add uv coord multiplier like Texture Coordinate node in UE
            uvSets = barycentric_interpolation(uv0, uv1, uv2, barycentrics);
        }

        float3x3 TBN = float3x3(1,0,0, 0,1,0, 0,0,1);

        [branch]
        if (model.vertexBufferTangents >= 0)
        {
            Buffer<float4> tangentBuffer = bindlessBuffersFloat4[descriptor_index(model.vertexBufferTangents)];
            float4 t0 = tangentBuffer[i0];
            float4 t1 = tangentBuffer[i1];
            float4 t2 = tangentBuffer[i2];
            t0.xyz = mul(adjointMat, t0.xyz);
            t1.xyz = mul(adjointMat, t1.xyz);
            t2.xyz = mul(adjointMat, t2.xyz);
            t0.xyz = any(t0.xyz) ? normalize(t0.xyz) : 0;
            t1.xyz = any(t1.xyz) ? normalize(t1.xyz) : 0;
            t2.xyz = any(t2.xyz) ? normalize(t2.xyz) : 0;
            T = barycentric_interpolation(t0, t1, t2, barycentrics);
            T.w = T.w < 0 ? -1 : 1;
            float3 B = cross(T.xyz, unnormalizedN) * T.w;   // Bitangent
            TBN = float3x3(T.xyz, B, unnormalizedN);

            [branch]
            if (material.textures[TEXTURE_SLOT_NORMAL].is_valid())
            {
                bumpColor = material.sample(TEXTURE_SLOT_NORMAL, uvSets, 0.0).xyz;
                bumpColor = bumpColor * 2 - 1;
            }
        }

        [branch]
        if (any(bumpColor))
        {
            N = normalize(mul(bumpColor, TBN));
        }

        if (material.textures[TEXTURE_SLOT_BASE_COLOR].is_valid())
        {
            baseColor = material.sample(TEXTURE_SLOT_BASE_COLOR, uvSets, 0.0);
        }
        else
        {
            baseColor = material.get_base_color();
        }

        if (material.textures[TEXTURE_SLOT_ROUGHNESS].is_valid())
        {
            roughness = material.sample(TEXTURE_SLOT_ROUGHNESS, uvSets, 0.0).r;
        }
        else
        {
            roughness = material.get_roughness();
        }

        roughness *= roughness;

        if (material.textures[TEXTURE_SLOT_METALLIC].is_valid())
        {
            metallic = material.sample(TEXTURE_SLOT_METALLIC, uvSets, 0.0).r;
        }
        else
        {
            metallic = material.get_metallic();
        }

        if (material.textures[TEXTURE_SLOT_AO].is_valid())
        {
            occlusion = material.sample(TEXTURE_SLOT_AO, uvSets, 0.0).r;
        }

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