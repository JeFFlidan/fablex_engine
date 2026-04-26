#ifndef SHADER_INTEROP_CAMERA
#define SHADER_INTEROP_CAMERA

#include "shader_interop_base.h"

struct ShaderCamera
{
	float3 position;
	uint empty1;

	float4 clipPlane;

	float3 forward;
	float zNear;

	float3 up;
	float zFar;

	float4x4 view;
	float4x4 projection;
	float4x4 viewProjection;
	float4x4 prevViewProjection;
	float4x4 inverseView;
	float4x4 inverseProjection;
	float4x4 inverseViewProjection;

	ShaderFrustum frustum;

#ifdef __cplusplus
	void create_frustum()
	{   
        using namespace DirectX;

		fe::Matrix projViewT = XMMatrixTranspose(XMLoadFloat4x4(&viewProjection));
		XMStoreFloat4(&frustum.planes[0], XMPlaneNormalize(XMVectorAdd(projViewT.data.r[3], projViewT.data.r[0])));
		XMStoreFloat4(&frustum.planes[1], XMPlaneNormalize(XMVectorSubtract(projViewT.data.r[3], projViewT.data.r[0])));
		XMStoreFloat4(&frustum.planes[2], XMPlaneNormalize(XMVectorAdd(projViewT.data.r[3], projViewT.data.r[1])));
		XMStoreFloat4(&frustum.planes[3], XMPlaneNormalize(XMVectorSubtract(projViewT.data.r[3], projViewT.data.r[1])));
		XMStoreFloat4(&frustum.planes[4], XMPlaneNormalize(projViewT.data.r[2]));
		XMStoreFloat4(&frustum.planes[5], XMPlaneNormalize(XMVectorSubtract(projViewT.data.r[3], projViewT.data.r[2])));
	}
#endif
};

static const uint MAX_CAMERA_COUNT = 16;

struct CameraUB
{
	ShaderCamera cameras[MAX_CAMERA_COUNT];
};

UNIFORM_BUFFER(cameraData, CameraUB, UB_CAMERA_SLOT);

#endif // SHADER_INTEROP_CAMERA