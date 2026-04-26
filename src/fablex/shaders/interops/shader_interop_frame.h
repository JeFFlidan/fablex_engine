#ifndef SHADER_INTEROP_FRAME
#define SHADER_INTEROP_FRAME

#include "shader_interop_base.h"

static const uint FRAME_FLAG_TEMPORAL_AA_ENABLED = 1 << 0;

struct alignas(16) FrameUB
{
	uint flags;
	float time;
	float previousTime;
	float deltaTime;

	uint frameCount;
	uint2 shadowMapAtlasExtent;
	int shadowMapAtlasIndex;

	int entityBufferIndex;
	int materialBufferIndex;
	int modelBufferIndex;
	int modelInstanceBufferIndex;
	int meshInstanceBufferIndex;
	int modelInstanceIDBufferIndex;
    int meshletInfoBufferIndex;

	uint lightArrayOffset;
	uint lightArrayCount;
    uint3 padding;
};


UNIFORM_BUFFER(frameData, FrameUB, UB_FRAME_SLOT);

#endif // SHADER_INTEROP_FRAME