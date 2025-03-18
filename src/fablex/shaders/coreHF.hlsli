#ifndef CORE_HF
#define CORE_HF

#ifdef CORE_SHADER_OUTPUT_PLANE
#define CORE_SHADER_USE_TEXCOORD
#define CORE_SHADER_USE_VERTEX_ID
#endif

struct VertexInput
{
    float3 location : SV_POSITION;
    uint instanceID : SV_InstanceID;
    
#ifdef CORE_SHADER_USE_NORMAL
    float3 normal : NORMAL;
#endif
    
#ifdef CORE_SHADER_USE_TANGENT
    float3 tangent : TANGENT;
#endif
    
#ifdef CORE_SHADER_USE_TEXCOORD
    float2 texCoord : TEXCOORD0;
#endif

#ifdef CORE_SHADER_USE_UV_SETS
    float4 uvSets : UVSETS;
#endif

#ifdef CORE_SHADER_USE_VERTEX_ID
    uint vertexID : SV_VertexID;
#endif
};

struct PixelInput
{
    precise float4 location : SV_POSITION;
    uint instanceID : SV_InstanceID;
    
#ifdef CORE_SHADER_USE_NORMAL
    float3 normal : NORMAL;
#endif
    
#ifdef CORE_SHADER_USE_TANGENT
    float3 tangent : TANGENT;
#endif
    
#ifdef CORE_SHADER_USE_TEXCOORD
    float2 texCoord : TEXCOORD0;
#endif
};

#endif // CORE_HF