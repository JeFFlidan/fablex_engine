#ifndef SHADER_INTEROP_CULLING
#define SHADER_INTEROP_CULLING

#include "shader_interop_base.h"

struct CullingDataUB
{
	float P00, P11, znear, zfar;
};

struct IDColor
{
	float4 color;
};

// Indirect drawing
struct ShaderModelInstanceID
{
	uint id;
	uint3 empty;
};

struct DrawIndexedIndirectCommand
{
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	uint vertexOffset;
	uint firstInstance;
};

struct CullingInstanceIndices
{
	uint objectID;
	uint batchID;
};

struct IndirectBufferPushConstants
{
	uint cullingInstanceIndicesBuffer;
	uint indirectCommandsBuffer;
	uint rendererModelInstanceIdBuffer;
	uint cullingParamsBuffer;
	uint cullingParams;
	float2 empty1;
};

struct CullingParams
{
	uint cameraIndex;
	uint depthPyramidIndex;
	float lodBase, lodStep;
	float pyramidWidth, pyramidHeight;

	uint drawCount;

	int isFrustumCullingEnabled;
	int isOcclusionCullingEnabled;
	int isAABBCheckEnabled;
	int isLodEnabled;
	float aabbmin_x;
	float aabbmin_y;
	float aabbmin_z;
	float aabbmax_x;
	float aabbmax_y;
	float aabbmax_z;
	float3 empty;
};

static const uint CULLING_GROUP_SIZE = 256;

#endif // SHADER_INTEROP_CULLING