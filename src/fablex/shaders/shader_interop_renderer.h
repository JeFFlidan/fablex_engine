#ifndef SHADER_INTEROP_RENDERER
#define SHADER_INTEROP_RENDERER

#include "shader_interop_base.h"

static const uint SHADER_MATERIAL_FLAG_TRANSPARENT = 1 << 0;
static const uint SHADER_MATERIAL_FLAG_UNLIT = 1 << 1;

enum TextureSlot
{
	TEXTURE_SLOT_BASE_COLOR = 0,
	TEXTURE_SLOT_NORMAL,
	TEXTURE_SLOT_ROUGHNESS,
	TEXTURE_SLOT_METALLIC,
	TEXTURE_SLOT_AO,

	TEXTURE_SLOT_COUNT
};

struct ShaderTexture2D
{
	int textureIndex;
	float3 empty;

	inline void init()
	{
		textureIndex = -1;
	}

	inline bool is_valid()
	{
		return textureIndex > 0;
	}

#ifndef __cplusplus
	Texture2D get_texture()
	{
		return bindlessTextures2DFloat4[NonUniformResourceIndex(textureIndex)];
	}

	float4 sample(in SamplerState sampler, in float4 uv_sets)
	{
		Texture2D texture = get_texture();
		float2 uv = uv_sets.xy;		// Have to think how to work with 2 uv sets
		return texture.Sample(sampler, uv);
	}
#endif
};

struct ShaderMaterial
{
	uint2 baseColor;	//half4
	uint roughness16Metallic16;
	uint opacity16SamplerIndex16;
	uint flags8;		// 24 bits are empty
	float3 empty;

	ShaderTexture2D textures[TEXTURE_SLOT_COUNT];

#ifndef __cplusplus
	void init()
	{
		uint albedoDefaultValue = f32tof16(1.0f);
		baseColor.x |= albedoDefaultValue;
		baseColor.x |= albedoDefaultValue << 16u;
		baseColor.y |= albedoDefaultValue;
		baseColor.y |= albedoDefaultValue << 16u;
		roughness16Metallic16 = f32tof16(0.5f);
		opacity16SamplerIndex16 = f32tof16(1.0f);
		flags8 = 0;
 	}

	inline float4 get_base_color()
	{
		return float4(
			f16tof32(baseColor.x),
			f16tof32(baseColor.x >> 16u),
			f16tof32(baseColor.y),
			f16tof32(baseColor.y >> 16u)
		);
	}

	inline float get_roughness()
	{
		return f16tof32(roughness16Metallic16);
	}

	inline float get_metallic()
	{
		return f16tof32(roughness16Metallic16 >> 16u);
	}

	inline uint get_sampler_index()
	{
		return opacity16SamplerIndex16 >> 16u;
	}

	void get_sampler(out SamplerState sampler)
	{
		uint samplerIndex = opacity16SamplerIndex16 >> 16u;
		sampler = bindlessSamplers[NonUniformResourceIndex(samplerIndex)];
	}

	inline float get_opacity()
	{
		return f16tof32(opacity16SamplerIndex16);
	}
#else
	void init()
	{
		uint albedoDefaultValue = DirectX::PackedVector::XMConvertFloatToHalf(1.0f);
		baseColor.x |= albedoDefaultValue;
		baseColor.x |= albedoDefaultValue << 16u;
		baseColor.y |= albedoDefaultValue;
		baseColor.y |= albedoDefaultValue << 16u;
		roughness16Metallic16 = DirectX::PackedVector::XMConvertFloatToHalf(0.5f);
		opacity16SamplerIndex16 = DirectX::PackedVector::XMConvertFloatToHalf(1.0f);
		flags8 = 0;
	}

	inline void set_base_color(float4 inBaseColor)
	{
		baseColor.x |= DirectX::PackedVector::XMConvertFloatToHalf(inBaseColor.x);
		baseColor.x |= DirectX::PackedVector::XMConvertFloatToHalf(inBaseColor.y) << 16u;
		baseColor.y |= DirectX::PackedVector::XMConvertFloatToHalf(inBaseColor.z);
		baseColor.y |= DirectX::PackedVector::XMConvertFloatToHalf(inBaseColor.w) << 16u;
	}

	inline void set_roughness(float roughness)
	{
		roughness16Metallic16 |= DirectX::PackedVector::XMConvertFloatToHalf(roughness);
	}

	inline void set_metallic(float metallic)
	{
		roughness16Metallic16 |= DirectX::PackedVector::XMConvertFloatToHalf(metallic) << 16u;
	}

	inline void set_opacity(float opacity)
	{
		opacity16SamplerIndex16 |= DirectX::PackedVector::XMConvertFloatToHalf(opacity);
	}

	inline void set_sampler_index(uint samplerIndex)
	{
		opacity16SamplerIndex16 |= samplerIndex << 16u;
	}

	inline void set_flags(uint flags)
	{
		flags8 |= (flags & 0xFF);
	}
#endif

	inline bool is_transparent()
	{
		uint flags = flags8 & 0xFF;
		return flags & SHADER_MATERIAL_FLAG_TRANSPARENT;
	}

	inline bool is_unlit()
	{
		uint flags = flags8 & 0xFF;
		return flags & SHADER_MATERIAL_FLAG_UNLIT;
	}
};

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

struct ShaderSphereBounds
{
	float3 center;
	float radius;

	void init()
	{
		center = float3(0.0f, 0.0f, 0.0f);
		radius = 0.0f;
	}
};

struct ShaderModelInstance
{
	ShaderTransform transform;	// With quantization mapping
	ShaderTransform rawTransform; // Without quantization mapping
	ShaderTransform transformInverseTranspose;	// Without quantization mapping
	
	ShaderSphereBounds sphereBounds;

	uint materialIndex;
	uint geometryOffset;

	float3 scale;

	void init()
	{
		transform.init();
		transformInverseTranspose.init();
		rawTransform.init();
		sphereBounds.init();

		materialIndex = 0;
		geometryOffset = 0;
		scale = float3(0.0f, 0.0f, 0.0f);
	}
};

enum ShaderEntityType
{
	SHADER_ENTITY_TYPE_UNDEFINED = 0,
	SHADER_ENTITY_TYPE_DIRECTIONAL_LIGHT,
	SHADER_ENTITY_TYPE_POINT_LIGHT,
	SHADER_ENTITY_TYPE_SPOT_LIGHT,
};

// Must be aligned to 16 bytes. For now ShaderEntity supports point, spot and directional lights
struct ShaderEntity
{
	float3 position;
	uint type8Flags8;					// 16 bits are empty
	
	uint2 direction16OuterConeAngle16;	// There are 16 empty bits if RenderEntity type is dir light. I think it can be used in the future
	uint2 color;						// half4
	
	uint matrixIndex16TextureIndex16;
	uint attenuationRadius16;			// 16 bits are empty
	uint extraInfo16;					// Can contain two half floats. For spot light it will contain info about angles. For point light it will contain info about cubemap 
	uint empty;							// Will be used in the future

	void init()
	{
		position = float3(0, 0, 0);
		type8Flags8 = 0;
		direction16OuterConeAngle16 = uint2(0, 0);
		color = uint2(0, 0);
		matrixIndex16TextureIndex16 = 0;
		attenuationRadius16 = 0;
		extraInfo16 = 0;
		empty = 0;
	}

#ifndef __cplusplus
	inline uint get_type()
	{
		return type8Flags8 & 0xFF;
	}

	inline uint get_flags()
	{
		return (type8Flags8 >> 8) & 0xFF;
	}

	// Returns normalized direction
	inline float3 get_direction()
	{
		return normalize(float3(
			f16tof32(direction16OuterConeAngle16.x),
			f16tof32(direction16OuterConeAngle16.x >> 16u),
			f16tof32(direction16OuterConeAngle16.y)
		));
	}

	inline float get_outer_cone_angle()
	{
		return f16tof32(direction16OuterConeAngle16.y >> 16u);
	}

	inline float4 get_color()
	{
		return float4(
			f16tof32(color.x),
			f16tof32(color.x >> 16u),
			f16tof32(color.y),
			f16tof32(color.y >> 16u)
		);
	}

	inline uint get_matrix_index()
	{
		return matrixIndex16TextureIndex16 & 0xFFFF;
	}

	inline uint get_texture_index()
	{
		return matrixIndex16TextureIndex16 >> 16u;
	}

	inline float get_attenuation_radius()
	{
		return f16tof32(attenuationRadius16);
	}

	inline float get_angle_scale()
	{
		return f16tof32(extraInfo16);
	}

	inline float get_angle_offset()
	{
		return f16tof32(extraInfo16 >> 16u);
	}
#else
	inline void set_type(uint type)
	{
		type8Flags8 |= (type & 0xFF);
	}

	inline void set_flags(uint flags)
	{
		type8Flags8 |= (flags & 0xFF) << 8u;
	}

	inline void set_direction(float3 direction)
	{
		direction16OuterConeAngle16.x = DirectX::PackedVector::XMConvertFloatToHalf(direction.x);
		direction16OuterConeAngle16.x = DirectX::PackedVector::XMConvertFloatToHalf(direction.y) << 16u;
		direction16OuterConeAngle16.y = DirectX::PackedVector::XMConvertFloatToHalf(direction.z);
	}

	inline void set_outer_cone_angle_cos(float angle)
	{
		direction16OuterConeAngle16.y |= DirectX::PackedVector::XMConvertFloatToHalf(angle) << 16u;
	}

	inline void set_color(float4 colorValue)
	{
		color.x |= DirectX::PackedVector::XMConvertFloatToHalf(colorValue.x);
		color.x |= DirectX::PackedVector::XMConvertFloatToHalf(colorValue.y) << 16u;
		color.y |= DirectX::PackedVector::XMConvertFloatToHalf(colorValue.z);
		color.y |= DirectX::PackedVector::XMConvertFloatToHalf(colorValue.w) << 16u;
	}

	inline void set_indices(uint matrixIndex, uint textureIndex)
	{
		matrixIndex16TextureIndex16 |= (matrixIndex & 0xFFFF);
		matrixIndex16TextureIndex16 |= (textureIndex & 0xFFFF) << 16u;
	}

	inline void set_attenuation_radius(float radius)
	{
		attenuationRadius16 |= DirectX::PackedVector::XMConvertFloatToHalf(radius);
	}

	// For spot lights
	inline void set_angle_scale(float angleScale)
	{
		extraInfo16 |= DirectX::PackedVector::XMConvertFloatToHalf(angleScale);
	}

	// For spot lights
	inline void set_angle_offset(float angleOffset)
	{
		extraInfo16 |= DirectX::PackedVector::XMConvertFloatToHalf(angleOffset) << 16u;
	}
#endif	// __cplusplus
};

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
	int modelInstanceIDBufferIndex;

	uint lightArrayOffset;
	uint lightArrayCount;
};

struct ShaderFrustum
{
	float4 planes[6];
#ifndef __cplusplus
	bool check(ShaderSphereBounds sphereBounds)
	{
		bool visible = true;
		[unroll]
		for (int i = 0; i != 6; ++i)
		{
			visible = visible && dot(planes[i], float4(sphereBounds.center, 1.0f)) > -sphereBounds.radius;
		}
		return visible;
	}
#endif
};

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

struct Viewport
{
	float left;
	float top;
	float right;
	float bottom;
};

static const uint MAX_CAMERA_COUNT = 16;

struct CameraUB
{
	ShaderCamera cameras[MAX_CAMERA_COUNT];
};

UNIFORM_BUFFER(frameData, FrameUB, UB_FRAME_SLOT);
UNIFORM_BUFFER(cameraData, CameraUB, UB_CAMERA_SLOT);

struct CullingDataUB
{
	float P00, P11, znear, zfar;
};

struct IDColor
{
	float4 color;
};

enum SamplerType
{
	SAMPLER_LINEAR_REPEAT = 0,
	SAMPLER_LINEAR_CLAMP,
	SAMPLER_LINEAR_MIRROR,

	SAMPLER_NEAREST_REPEAT,
	SAMPLER_NEAREST_CLAMP,
	SAMPLER_NEAREST_MIRROR,

	SAMPLER_MINIMUM_NEAREST_CLAMP,

	SAMPLER_COUNT
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

// ============= MESHLETS =============

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

static const uint CULLING_GROUP_SIZE = 256;

struct DepthReduceData
{
	uint levelWidth;
	uint levelHeight;
	uint inDepthTextureIndex;
	uint outDepthTextureIndex;
};

static const uint DEPTH_REDUCE_GROUP_SIZE = 32;

// ============= PUSH CONSTANTS =============

// In push constants input texture descriptors must go first, after them must be storage output texture descriptors.
// This is necessary to automatically fill some fields of push constants.
// Not best approach, but I will use it for now not to think about more robust way to fill push constants.

struct TriangleSwapChainPushConstants
{
	uint triangleTextureIndex;
	float3 alignment;
};

struct ObjectPushConstants
{
	uint modelIndex;
	uint instanceOffset;
	uint2 padding;
};

struct RayTracingPushConstants
{
	uint outputTargetIndex;
	uint tlasIndex;
	uint2 padding;
};

#endif // SHADER_INTEROP_RENDERER