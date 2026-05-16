#define OBJECT_GBUFFER
#include "gbuffer/vbuffer.hlsli"
#include "interops/shader_interop_meshlet.h"

PUSH_CONSTANTS(cb, GBufferPushConstants)

struct SharedData
{
    float clipPositionsWComponent[MESHLET_VERTEX_COUNT];
    float2 screenSpacePositions[MESHLET_VERTEX_COUNT];
    float2 uvs[MESHLET_VERTEX_COUNT];
    float3 normals[MESHLET_VERTEX_COUNT];
    float4 tangents[MESHLET_VERTEX_COUNT];
    float4x4 modelViewProj;
    float3x3 adjoint;
    uint modelIndex;
    uint materialIndex;
};

groupshared SharedData g_SharedData;
groupshared uint g_FirstInstanceID;
groupshared uint g_FirstMeshletID;
groupshared bool g_IsSameMeshlet;

struct TriData
{
    float2 sc[3]; // screen space positions
    float w[3];   // perspective values
    float2 uv[3]; // uv values
    float3 n[3];  // normal values
    float4 t[3];  // tangent values

    float2 pixelPos;
    uint2 pixelToWrite;
    uint materialIndex;
};

void write_pixel_data(in TriData triData)
{
    float3 invW = inverse_perspective(triData.w[0], triData.w[1], triData.w[2]);
    float4 baryAndInterpInvW = compute_perspective_correct_barycentrics(
        triData.pixelPos, triData.sc[0], triData.sc[1], triData.sc[2], invW);

    float4 pixelUV = float4(0, 0, 0, 0);
    pixelUV.xy = perspective_barycentric_interpolation(triData.uv[0], triData.uv[1], triData.uv[2], baryAndInterpInvW);
    
    float3 pixelNormal = perspective_barycentric_interpolation(triData.n[0], triData.n[1], triData.n[2], baryAndInterpInvW);
    float4 pixelTangent = perspective_barycentric_interpolation(triData.t[0], triData.t[1], triData.t[2], baryAndInterpInvW);
    pixelTangent.w = pixelTangent.w < 0 ? -1 : 1;

    float3 pixelBitangent = cross(pixelTangent.xyz, pixelNormal) * pixelTangent.w;
    float3x3 TBN = float3x3(pixelTangent.xyz, pixelBitangent, pixelNormal);

    ShaderMaterial material = get_material(triData.materialIndex);
    float4 albedo = material.get_base_color();

    if (material.textures[TEXTURE_SLOT_BASE_COLOR].is_valid())
    {
        // Use mip 0 to avoid derivative issues in Compute Shaders
        albedo *= material.sample(TEXTURE_SLOT_BASE_COLOR, pixelUV, 0);
    }

    if (material.textures[TEXTURE_SLOT_NORMAL].is_valid())
    {
        float3 bumpColor = material.sample(TEXTURE_SLOT_NORMAL, pixelUV, 0.0).xyz;
        bumpColor = bumpColor * 2 - 1;

        if (any(bumpColor))
        {
            pixelNormal = normalize(mul(bumpColor, TBN));
        }
    }
    else
    {
        pixelNormal = normalize(pixelNormal);
    }

    float4 pixelSurface = float4(0, 0, 0, 0);
    if (material.textures[TEXTURE_SLOT_ROUGHNESS].is_valid())
    {
        pixelSurface.r = material.sample(TEXTURE_SLOT_ROUGHNESS, pixelUV, 0.0).r;
        pixelSurface.r *= material.get_roughness();
    }
    else
    {
        pixelSurface.r = material.get_roughness();
    }

    if (material.textures[TEXTURE_SLOT_METALLIC].is_valid())
    {
        pixelSurface.g = material.sample(TEXTURE_SLOT_METALLIC, pixelUV, 0.0).r;
        pixelSurface.g *= material.get_metallic();
    }
    else
    {
        pixelSurface.g = material.get_metallic();
    }

    if (material.textures[TEXTURE_SLOT_AO].is_valid())
    {
        pixelSurface.b = material.sample(TEXTURE_SLOT_AO, pixelUV, 0.0).r;
    }

    if (material.textures[TEXTURE_SLOT_ARM].is_valid())
    {
        float4 value = material.sample(TEXTURE_SLOT_ARM, pixelUV, 0.0);
        pixelSurface.r = value.g * material.get_roughness();
        pixelSurface.g = value.b * material.get_metallic();
        pixelSurface.b = value.r;
    }

    cb.outColor.write(triData.pixelToWrite, albedo);

    float4 finalNormaVelocityOutput = float4(ndir_to_oct_snorm(pixelNormal), 0, 0);
    cb.outNormal.write(triData.pixelToWrite, finalNormaVelocityOutput);

    cb.outSurface.write(triData.pixelToWrite, pixelSurface);
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    uint2 packedData = bindlessTextures2DUInt2[cb.inVBuffer.descriptor][DTid.xy];
    VBufferData pixelData = unpack_vbuffer_data(packedData);

    bool isBackground = pixelData.instanceID == 1203982336;

    if (groupIndex == 0)
    {
        g_FirstInstanceID = pixelData.instanceID;
        g_FirstMeshletID = pixelData.meshletID;
        g_IsSameMeshlet = true;
    }

    GroupMemoryBarrierWithGroupSync();

    if (pixelData.meshletID != g_FirstMeshletID || pixelData.instanceID != g_FirstInstanceID)
    {
        g_IsSameMeshlet = false;
    }

    GroupMemoryBarrierWithGroupSync();

    if (g_IsSameMeshlet && !isBackground)
    {
        if (groupIndex == 0)
        {
            ShaderModelInstance instance = get_model_instance(pixelData.instanceID);
            ShaderMeshletInfo meshletInfo = get_meshlet_info(instance.meshletOffset + pixelData.meshletID);
            ShaderMeshInstance meshInstance = get_mesh_instance(meshletInfo.modelMeshID);

            g_SharedData.modelIndex = meshInstance.modelIndex;
            g_SharedData.materialIndex = meshInstance.materialIndex;
            g_SharedData.modelViewProj = mul(get_camera().viewProjection, instance.transform.get_matrix());
            g_SharedData.adjoint = instance.rawTransform.get_matrix_adjoint();
        }

        GroupMemoryBarrierWithGroupSync();

        ShaderModel model = get_model(g_SharedData.modelIndex);
        ShaderMeshlet meshlet = bindlesStructuredMeshlets[model.vertexBufferMeshlets][g_FirstMeshletID];
        float2 screenRes = float2(1920, 1080);

        for (uint i = groupIndex; i < meshlet.vertexCount; i += 64)
        {
            uint vertexIndex = meshlet.vertices[i];

            float4 localPos = bindlessBuffersFloat4[model.vertexBufferPosWind][vertexIndex];
            float4 clipSpacePos = mul(g_SharedData.modelViewProj, float4(localPos.xyz, 1.0));

            float3 ndc = clipSpacePos.xyz / clipSpacePos.w;

            float screenSpaceX = (ndc.x * 0.5 + 0.5) * screenRes.x;
            float screenSpaceY = (ndc.y * -0.5 + 0.5) * screenRes.y;

            g_SharedData.screenSpacePositions[i] = float2(screenSpaceX, screenSpaceY);
            g_SharedData.clipPositionsWComponent[i] = clipSpacePos.w;

            float2 uv = bindlessBuffersFloat4[model.vertexBufferUVs][vertexIndex].xy;

            g_SharedData.uvs[i] = lerp(model.uvRangeMin.xy, model.uvRangeMax.xy, uv);

            if (model.vertexBufferNormals >= 0)
            {
                Buffer<float4> normalBuffer = bindlessBuffersFloat4[descriptor_index(model.vertexBufferNormals)];
                float3 n = mul(g_SharedData.adjoint, normalBuffer[vertexIndex].xyz);
                g_SharedData.normals[i] = any(n) ? normalize(n) : 0;
            }

            if (model.vertexBufferTangents >= 0)
            {
                Buffer<float4> tangentBuffer = bindlessBuffersFloat4[descriptor_index(model.vertexBufferTangents)];
                float4 t = tangentBuffer[vertexIndex];
                t.xyz = mul(g_SharedData.adjoint, tangentBuffer[vertexIndex].xyz);
                t.xyz = any(t.xyz) ? normalize(t.xyz) : 0;
                g_SharedData.tangents[i] = t;
            }
        }

        GroupMemoryBarrierWithGroupSync();

        uint3 localIndices = meshlet.triangles[pixelData.triangleID].tri();

        TriData triData;

        triData.sc[0] = g_SharedData.screenSpacePositions[localIndices.x];
        triData.sc[1] = g_SharedData.screenSpacePositions[localIndices.y];
        triData.sc[2] = g_SharedData.screenSpacePositions[localIndices.z];
        
        triData.w[0] = g_SharedData.clipPositionsWComponent[localIndices.x];
        triData.w[1] = g_SharedData.clipPositionsWComponent[localIndices.y];
        triData.w[2] = g_SharedData.clipPositionsWComponent[localIndices.z];

        triData.uv[0] = g_SharedData.uvs[localIndices.x];
        triData.uv[1] = g_SharedData.uvs[localIndices.y];
        triData.uv[2] = g_SharedData.uvs[localIndices.z];

        triData.n[0] = g_SharedData.normals[localIndices.x];
        triData.n[1] = g_SharedData.normals[localIndices.y];
        triData.n[2] = g_SharedData.normals[localIndices.z];

        triData.t[0] = g_SharedData.tangents[localIndices.x];
        triData.t[1] = g_SharedData.tangents[localIndices.y];
        triData.t[2] = g_SharedData.tangents[localIndices.z];

        triData.pixelPos = float2(DTid.xy) + 0.5;
        triData.pixelToWrite = DTid.xy;
        triData.materialIndex = g_SharedData.materialIndex;

        write_pixel_data(triData);
    }
    else if (!isBackground)
    {
        ShaderModelInstance instance = get_model_instance(pixelData.instanceID);
        ShaderMeshletInfo meshletInfo = get_meshlet_info(instance.meshletOffset + pixelData.meshletID);
        ShaderMeshInstance meshInstance = get_mesh_instance(meshletInfo.modelMeshID);
        ShaderModel model = get_model(meshInstance.modelIndex);
        ShaderMeshlet meshlet = bindlesStructuredMeshlets[model.vertexBufferMeshlets][pixelData.meshletID];

        uint3 localIndices = meshlet.triangles[pixelData.triangleID].tri();
        uint vIdx[3] = { meshlet.vertices[localIndices.x], meshlet.vertices[localIndices.y], meshlet.vertices[localIndices.z] };

        TriData triData;

        float4x4 modelMatrix = instance.transform.get_matrix();
        float4x4 viewProj = get_camera().viewProjection;
        float4x4 mvp = mul(viewProj, modelMatrix);
        float3x3 adjoint = instance.rawTransform.get_matrix_adjoint();
        float2 screenRes = float2(1920, 1080); 

        for(int i = 0; i < 3; i++) {
            float4 localPos = bindlessBuffersFloat4[model.vertexBufferPosWind][vIdx[i]];
            float4 clipSpacePos = mul(mvp, float4(localPos.xyz, 1.0));
            
            float3 ndc = clipSpacePos.xyz / clipSpacePos.w;

            triData.sc[i].x = (ndc.x * 0.5 + 0.5) * screenRes.x;
            triData.sc[i].y = (ndc.y * -0.5 + 0.5) * screenRes.y; 

            triData.w[i] = clipSpacePos.w;

            float2 uv = bindlessBuffersFloat4[model.vertexBufferUVs][vIdx[i]].xy;

            triData.uv[i] = lerp(model.uvRangeMin.xy, model.uvRangeMax.xy, uv);

            if (model.vertexBufferNormals >= 0)
            {
                Buffer<float4> normalBuffer = bindlessBuffersFloat4[descriptor_index(model.vertexBufferNormals)];
                float3 n = mul(adjoint, normalBuffer[vIdx[i]].xyz);
                triData.n[i] = any(n) ? normalize(n) : 0;
            }

            if (model.vertexBufferTangents >= 0)
            {
                Buffer<float4> tangentBuffer = bindlessBuffersFloat4[descriptor_index(model.vertexBufferTangents)];
                float4 t = tangentBuffer[vIdx[i]];
                t.xyz = mul(adjoint, tangentBuffer[vIdx[i]].xyz);
                t.xyz = any(t.xyz) ? normalize(t.xyz) : 0;
                triData.t[i] = t;
            }
        }

        triData.pixelPos = float2(DTid.xy) + 0.5;
        triData.pixelToWrite = DTid.xy;
        triData.materialIndex = meshInstance.materialIndex;

        write_pixel_data(triData);
    }
    else
    {
        cb.outColor.write(DTid.xy, float4(0.1, 0.1, 0.1, 1.0));
        cb.outNormal.write(DTid.xy, float4(0, 0, 0, 1));
        cb.outSurface.write(DTid.xy, float4(0, 0, 0, 1));
    }
}
