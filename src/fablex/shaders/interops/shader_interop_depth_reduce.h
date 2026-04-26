#ifndef SHADER_INTEROP_DEPTH_REDUCE
#define SHADER_INTEROP_DEPTH_REDUCE

#include "shader_interop_base.h"

struct DepthReduceData
{
	uint levelWidth;
	uint levelHeight;
	uint inDepthTextureIndex;
	uint outDepthTextureIndex;
};

static const uint DEPTH_REDUCE_GROUP_SIZE = 32;

#endif // SHADER_INTEROP_DEPTH_REDUCE