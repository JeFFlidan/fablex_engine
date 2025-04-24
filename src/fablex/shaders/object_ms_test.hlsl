#define OBJECT_SHADER_USE_COLOR
#include "object_hf.hlsli"

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void main(
    uint gid : SV_GroupID,
    uint gtid : SV_GroupThreadID,
    out indices uint3 triangles[MESHLET_TRIANGLE_COUNT],
    out vertices PixelInput vertices[MESHLET_VERTEX_COUNT]
)
{
    ShaderModelInstance modelInstance = get_model_instance(0);
    ShaderModel model = get_model(modelInstance.geometryOffset);

    ShaderMeshlet meshlet = bindlesStructuredMeshlets[model.vertexBufferMeshlets][gid];
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
        vertexInput.instanceID = 0;
        vertexInput.vertexID = vertexIndex;

        vertices[gtid] = create_pixel_input(vertexInput);

        vertices[gtid].color = float3(
            float(gid & 1),
            float(gid & 3) / 4,
            float(gid & 7) / 8
        );
    }
}
