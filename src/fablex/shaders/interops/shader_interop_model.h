#ifndef SHADER_INTEROP_MODEL
#define SHADER_INTEROP_MODEL

#include "shader_interop_base.h"

struct ShaderModel
{
	int indexBuffer;
	int vertexBufferPosWind;
	int vertexBufferMeshlets;
	int vertexBufferMeshletBounds;
	int vertexBufferNormals;
	int vertexBufferTangents;
	int vertexBufferUVs;
	int vertexBufferAtlas;
	int vertexBufferColors;

	float2 uvRangeMin;
	float2 uvRangeMax;

	float3 aabbMin;
	float3 aabbMax;

	void init()
	{
		indexBuffer = -1;
		vertexBufferPosWind = -1;
		vertexBufferNormals = -1;
		vertexBufferTangents = -1;
		vertexBufferUVs = -1;
		vertexBufferAtlas = -1;
		vertexBufferColors = -1;

		uvRangeMin = float2(0.0f, 0.0f);
		uvRangeMax = float2(1.0f, 1.0f);

		aabbMin = float3(0.0f, 0.0f, 0.0f);
		aabbMax = float3(0.0f, 0.0f, 0.0f);
	}
};

struct ShaderTransform
{
	float4 location;
	float4 rotation;
	float4 scale;

	void init()
	{
		location = float4(1.0f, 0.0f, 0.0f, 0.0f);
		rotation = float4(0.0f, 1.0f, 0.0f, 0.0f);
		scale = float4(0.0f, 0.0f, 1.0f, 0.0f);
	}

	void set_transfrom(float4x4 mat)
	{
		location = float4(mat._11, mat._21, mat._31, mat._41);
		rotation = float4(mat._12, mat._22, mat._32, mat._42);
		scale = float4(mat._13, mat._23, mat._33, mat._43);
	}

	float4x4 get_matrix()
	{
		return float4x4(
			location.x, location.y, location.z, location.w,
			rotation.x, rotation.y, rotation.z, rotation.w,
			scale.x, scale.y, scale.z, scale.w,
			0.0f, 0.0f, 0.0f, 1.0f
		);
	}

#ifndef __cplusplus
	float3x3 get_matrix_adjoint()
	{
		return adjoint(get_matrix());
	}
#endif // __cplusplus
};

struct ShaderModelInstance
{
	ShaderTransform transform;	// With quantization mapping
	ShaderTransform rawTransform; // Without quantization mapping
	ShaderTransform prevTransform;
	ShaderTransform transformInverseTranspose;	// Without quantization mapping
	
	ShaderSphereBounds sphereBounds;

	uint materialIndex;
	uint meshOffset;
    uint meshCount;
    uint meshletOffset;

	float3 scale;
    uint padding1;

	void init()
	{
		transform.init();
		transformInverseTranspose.init();
		rawTransform.init();
		prevTransform.init();
		sphereBounds.init();

		materialIndex = 0;
		meshOffset = 0;
        meshCount = 0;
		scale = float3(0.0f, 0.0f, 0.0f);
	}
};

struct ShaderMeshInstance
{
	uint materialIndex;
	uint modelIndex;
	uint indexOffset;
	uint meshletCount;
    uint meshletOffset;
    uint3 padding;

	void init()
	{
		materialIndex = 0;
		modelIndex = 0;
		indexOffset = 0;
        meshletCount = 0;
        meshletOffset = 0;
		padding = uint3(0, 0, 0);
	}
};

#endif // SHADER_INTEROP_MODEL