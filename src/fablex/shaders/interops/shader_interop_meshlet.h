#ifndef SHADER_INTEROP_MESHLET
#define SHADER_INTEROP_MESHLET

#include "shader_interop_base.h"

static const uint MESHLET_VERTEX_COUNT = 64u;
static const uint MESHLET_TRIANGLE_COUNT = 124u;

struct ShaderMeshletTriangle
{
	uint data;

	void init(uint i0, uint i1, uint i2, uint flags = 0)
	{
		data = 0;
		data |= i0 & 0xFF;
		data |= (i1 & 0xFF) << 8u;
		data |= (i2 & 0xFF) << 16u;
		data |= (flags & 0xFF) << 24u;
	}

	uint i0() { return data & 0xFF; }
	uint i1() { return (data >> 8u) & 0xFF; }
	uint i2() { return (data >> 16u) & 0xFF; }
	uint flags() { return (data >> 24u) & 0xFF; }
	uint3 tri() { return uint3(i0(), i1(), i2()); }
};

struct ShaderMeshlet
{
	uint vertexCount;
	uint triangleCount;
	uint2 padding;

	uint vertices[MESHLET_VERTEX_COUNT];
	ShaderMeshletTriangle triangles[MESHLET_TRIANGLE_COUNT];
};

struct ShaderMeshletBounds
{
	ShaderSphereBounds bounds;
	float3 coneAxis;
	float coneCutoff;
};

struct ShaderMeshletInfo
{
	uint modelInstanceID;
	uint modelMeshID;
	uint2 padding;
};

#endif // SHADER_INTEROP_MESHLET
