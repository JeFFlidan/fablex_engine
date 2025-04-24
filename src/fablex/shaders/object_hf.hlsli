#ifndef OBJECT_HF
#define OBJECT_HF

#include "common.hlsli"

#ifdef OBJECT_SHADER_BASE_MODEL_LAYOUT
#define OBJECT_SHADER_USE_NORMAL
#define OBJECT_SHADER_USE_TANGENT
#define OBJECT_SHADER_USE_TEXCOORD
#endif

#ifdef OBJECT_SHADER_OUTPUT_PLANE
#define OBJECT_SHADER_USE_UV_SETS
#define OBJECT_SHADER_USE_VERTEX_ID
#endif

struct VertexInput
{
    uint instanceID : SV_InstanceID;
    uint vertexID : SV_VertexID;

    ShaderModelInstance get_model_instance()
    {
        return ::get_model_instance(instanceID);
    }

    ShaderModel get_model()
    {
        return ::get_model(get_model_instance().geometryOffset);
    }

    float4 get_position_wind()
    {
        return bindlessBuffersFloat4[get_model().vertexBufferPosWind][vertexID];
    }

    float3 get_normal()
    {
        [branch]
        if (get_model().vertexBufferNormals < 0)
            return float3(0.0f, 0.0f, 0.0f);
        
        return bindlessBuffersFloat4[get_model().vertexBufferNormals][vertexID].xyz;
    }

    float4 get_tangent()
    {
        [branch]
        if (get_model().vertexBufferTangents < 0)
            return float4(0.0f, 0.0f, 0.0f, 0.0f);

        return bindlessBuffersFloat4[get_model().vertexBufferTangents][vertexID];
    }

    float4 get_uv_sets()
    {
        [branch]
        if (get_model().vertexBufferUVs < 0)
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
        
        return bindlessBuffersFloat4[get_model().vertexBufferUVs][vertexID];
    }
};

struct VertexDesc
{
    float4 position;
    float3 normal;
    float4 tangent;
    float4 uvSets;

    void init(VertexInput input)
    {
        ShaderModelInstance modelInstance = input.get_model_instance();

        float4 posWind = input.get_position_wind();
        position = float4(posWind.xyz, 1.0f);
        position = mul(modelInstance.transform.get_matrix(), position);

        float3x3 adjointMat = modelInstance.rawTransform.get_matrix_adjoint();

        normal = input.get_normal();
        normal = mul(adjointMat, normal);
        normal = any(normal) ? normalize(normal) : 0;

        tangent = input.get_tangent();
        tangent.xyz = mul(adjointMat, tangent.xyz);
        tangent.xyz = any(tangent.xyz) ? normalize(tangent.xyz) : 0;

        uvSets = input.get_uv_sets();
    }
};

struct PixelInput
{
    precise float4 position : SV_POSITION;

#ifdef OBJECT_SHADER_USE_INSTANCE_ID
    uint instanceID : SV_InstanceID;
#endif // OBJECT_SHADER_USE_INSTANCE_ID
    
#ifdef OBJECT_SHADER_USE_NORMAL
    float3 normal : NORMAL;
#endif // OBJECT_SHADER_USE_NORMAL
    
#ifdef OBJECT_SHADER_USE_TANGENT
    float4 tangent : TANGENT;
#endif // OBJECT_SHADER_USE_TANGENT
    
#ifdef OBJECT_SHADER_USE_UV_SETS
    float4 uvSets : TEXCOORD0;
#endif // OBJECT_SHADER_USE_UV_SETS

#ifdef OBJECT_SHADER_USE_COLOR
    float3 color : COLOR;
#endif // OBJECT_SHADER_USE_COLOR

    void init(VertexDesc vertDesc)
    {
        ShaderCamera camera = get_camera();

        position = mul(camera.viewProjection, vertDesc.position);

#ifdef OBJECT_SHADER_USE_NORMAL
        normal = vertDesc.normal;
#endif // OBJECT_SHADER_USE_NORMAL

#ifdef OBJECT_SHADER_USE_TANGENT
        tangent = vertDesc.tangent;
#endif // OBJECT_SHADER_USE_TANGENT

#ifdef OBJECT_SHADER_USE_UV_SETS
        uvSets = vertDesc.uvSets;
#endif // OBJECT_SHADER_USE_UV_SETS
    }
};

PixelInput create_pixel_input(in VertexInput vertInput)
{
    VertexDesc vertDesc;
    vertDesc.init(vertInput);

    PixelInput output;
    output.init(vertDesc);
    return output;
}

#ifdef OBJECT_SHADER_COMPILE_VS
PixelInput main(VertexInput vertInput)
{
    return create_pixel_input(vertInput);
}
#endif // OBJECT_SHADER_COMPILE_VS

#endif // OBJECT_HF