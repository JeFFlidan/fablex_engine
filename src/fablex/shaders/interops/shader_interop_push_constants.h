#ifndef SHADER_INTEROP_PUSH_CONSTANTS
#define SHADER_INTEROP_PUSH_CONSTANTS

#include "shader_interop_descriptor.h"

#ifdef __cplusplus
#define DEFINE_PUSH_CONSTANTS(Type) static constexpr const char* TypeName = #Type;
#else
#define DEFINE_PUSH_CONSTANTS(Type)
#endif	// __cplusplus

struct SwapChainPushConstants
{
	DEFINE_PUSH_CONSTANTS(SwapChainPushConstants);

	Texture2D_Descriptor<float4> resultTexture;
	float3 alignment;
};

struct ObjectPushConstants
{
	DEFINE_PUSH_CONSTANTS(ObjectPushConstants);

	uint modelIndex;
	uint instanceOffset;
	uint2 padding;
};

struct RayTracingPushConstants
{
	DEFINE_PUSH_CONSTANTS(RayTracingPushConstants);

	RWTexture2D_Descriptor<float4> outColor;
	AccelerationStructure_Descriptor tlas;
	uint2 padding;
};

struct PathTracingPushConstants
{
	DEFINE_PUSH_CONSTANTS(PathTracingPushConstants);

	RWTexture2D_Descriptor<float4> outColor;
	RWTexture2D_Descriptor<float2> outMotionVector;
	RWTexture2D_Descriptor<float4> outAlbedo;
	RWTexture2D_Descriptor<float4> outEmission;
	RWTexture2D_Descriptor<float4> outDepthNormal;
	Texture2D_Descriptor<float4> inPrevDepthNormal;
	RWTexture2D_Descriptor<float4> outIllumination;
	Texture2D_Descriptor<float4> inPrevIllumination;
	RWTexture2D_Descriptor<float2> outMoments;
	Texture2D_Descriptor<float2> inPrevMoments;
	RWTexture2D_Descriptor<float> outHistoryLength;
	Texture2D_Descriptor<float> inPrevHistoryLength;

	AccelerationStructure_Descriptor tlas;
	
	uint bounceCount;
	uint frameNumber;
	float accumulationFactor;
	float alpha;
	float momentsAlpha;
};

struct SVGFFilterMomentsPushConstants
{
	DEFINE_PUSH_CONSTANTS(SVGFFilterMomentsPushConstants);

	RWTexture2D_Descriptor<float4> outFilteredIllumination;
	Texture2D_Descriptor<float4> inIllumination;
	Texture2D_Descriptor<float2> inMoments;
	Texture2D_Descriptor<float> inHistoryLength;
	Texture2D_Descriptor<float4> inDepthNormal;

	float phiColor;
	float phiNormal;
};

struct SVGFAtrousPushConstants
{
	DEFINE_PUSH_CONSTANTS(SVGFAtrousPushConstants);

	RWTexture2D_Descriptor<float4> outFinalIllumination;
	Texture2D_Descriptor<float4> inAlbedo;
	RWTexture2D_Descriptor<float4> inIllumination;
	Texture2D_Descriptor<float> inHistoryLength;
	Texture2D_Descriptor<float4> inDepthNormal;

	int stepSize;
	float phiColor;
	float phiNormal;
};

struct SVGFFinalModulatePushConstants
{
	DEFINE_PUSH_CONSTANTS(SVGFFinalModulatePushConstants);

	RWTexture2D_Descriptor<float4> outColor;
	Texture2D_Descriptor<float4> inIllumination;
	Texture2D_Descriptor<float4> inAlbedo;
	Texture2D_Descriptor<float4> inEmission;
};

struct RNGSeedGenerationPushConstants
{
	DEFINE_PUSH_CONSTANTS(RNGSeedGenerationPushConstants);

	uint rngSeedTextureIndex;
	uint frameNumber;
	uint blueNoiseTextureSize;
	uint blueNoiseTextureDepth;
};

struct GBufferPushConstants
{
	DEFINE_PUSH_CONSTANTS(GBufferPushConstants);

	Texture2D_Descriptor<uint2> inVBuffer;
	RWTexture2D_Descriptor<float4> outColor;
	RWTexture2D_Descriptor<float4> outNormal;
	RWTexture2D_Descriptor<float4> outSurface;
	uint padding;
};

struct ImGuiPushConstants
{
	float2 scale;
	float2 translate;
	Sampler_Descriptor sampler;
	Texture2D_Descriptor<float4> texture;
};

#endif // SHADER_INTEROP_PUSH_CONSTANTS