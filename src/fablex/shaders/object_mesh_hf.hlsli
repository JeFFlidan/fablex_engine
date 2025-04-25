#ifndef OBJECT_MESH_HF
#define OBJECT_MESH_HF

#ifdef OBJECT_MESH_SHADER_DEBUG
#define OBJECT_MESH_SHADER_USE_MESHLET_COLORS
#endif // OBJECT_MESH_SHADER_DEBUG

#ifdef OBJECT_MESH_SHADER_USE_MESHLET_COLORS
#define OBJECT_SHADER_USE_COLOR
#endif // OBJECT_MESH_SHADER_USE_MESHLET_COLORS

#include "object_hf.hlsli"

static const uint TS_GROUP_SIZE = 32;
static const uint MS_GROUP_SIZE = 128;

struct Payload
{
    uint instanceID;
    uint meshletGroupOffset;
    uint meshletIndices[TS_GROUP_SIZE];
};

#ifdef OBJECT_MESH_SHADER_COMPILE_TS

groupshared Payload payload;

[numthreads(TS_GROUP_SIZE, 1, 1)]
void main(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupThreadID,
    uint gIdx : SV_GroupIndex
)
{
    uint instanceID = gid.y;
    uint meshletGroupOffset = gid.x * TS_GROUP_SIZE;

    if (WaveIsFirstLane())
    {
        payload.instanceID = instanceID;
        payload.meshletGroupOffset = meshletGroupOffset;
    }

    bool visible = true;   // TEMP

    if (visible)
    {
        uint index = WavePrefixCountBits(visible);
        payload.meshletIndices[index] = gIdx;
    }

    uint visibleCount = WaveActiveCountBits(visible);

    DispatchMesh(visibleCount, 1, 1, payload);
}

#endif // OBJECT_MESH_SHADER_COMPILE_TS

#ifdef OBJECT_MESH_SHADER_COMPILE_MS

[outputtopology("triangle")]
[numthreads(MS_GROUP_SIZE, 1, 1)]
void main(
    uint gid : SV_GroupID,
    uint gtid : SV_GroupThreadID,
    in payload Payload payload,
    out indices uint3 triangles[MESHLET_TRIANGLE_COUNT],
    out vertices PixelInput vertices[MESHLET_VERTEX_COUNT]
)
{
    ShaderModelInstance modelInstance = get_model_instance(payload.instanceID);
    ShaderModel model = get_model(pushConstants.modelIndex);

    uint meshletIndex = payload.meshletGroupOffset + payload.meshletIndices[gid];
    ShaderMeshlet meshlet = bindlesStructuredMeshlets[model.vertexBufferMeshlets][meshletIndex];
    SetMeshOutputCounts(meshlet.vertexCount, meshlet.triangleCount);

    [branch]
    if (gtid < meshlet.triangleCount)
    {
        triangles[gtid] = meshlet.triangles[gtid].tri();
    }

    [branch]
    if (gtid < meshlet.vertexCount)
    {
        uint vertexIndex = meshlet.vertices[gtid];
        VertexInput vertexInput;
        vertexInput.instanceID = payload.instanceID;
        vertexInput.vertexID = vertexIndex;

        vertices[gtid] = create_pixel_input(vertexInput);

        vertices[gtid].color = float3(
            float(gid & 1),
            float(gid & 3) / 4,
            float(gid & 7) / 8
        );
    }
}

#endif // OBJECT_MESH_SHADER_COMPILE_MS

#endif // OBJECT_MESH_HF