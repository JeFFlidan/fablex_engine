#include "common.hlsli"

static const uint THREADCOUNT = 64;

[numthreads(1, THREADCOUNT, 1)]
void main(uint3 Gid : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
	uint instanceIndex = Gid.x;
	ShaderModelInstance model = get_model_instance(instanceIndex);

	for (uint i = 0; i < model.meshCount; ++i)
	{
		uint meshIndex = model.meshOffset + i;
		ShaderMeshInstance mesh = get_mesh_instance(meshIndex);

		for (uint j = groupIndex; j < mesh.meshletCount; j += THREADCOUNT)
		{
			ShaderMeshletInfo meshletInfo = (ShaderMeshletInfo)0;
			meshletInfo.modelInstanceID = instanceIndex;
			meshletInfo.modelMeshID = meshIndex;

			uint meshletIndex = mesh.meshletOffset + j;
			update_meshlet_info(meshletInfo, meshletIndex);
		}
	}
}
