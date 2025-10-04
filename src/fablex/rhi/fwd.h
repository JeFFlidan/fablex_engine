#pragma once

#include "core/types.h"

namespace fe::rhi
{

struct RHIInitInfo;
struct BufferInfo;
struct Buffer;
struct TextureInfo;
struct Texture;
struct TextureInitInfo;
struct TextureViewInfo;
struct TextureView;
struct BufferViewInfo;
struct BufferView;
struct CommandPoolInfo;
struct CommandPool;
struct CommandBufferInfo;
struct CommandBuffer;
struct Fence;
struct Semaphore;
struct SwapChainInfo;
struct SwapChain;
struct ShaderInfo;
struct Shader;
struct SamplerInfo;
struct Sampler;
struct GraphicsPipelineInfo;
struct ComputePipelineInfo;
struct RayTracingPipelineInfo;
struct Pipeline;
struct AccelerationStructureInfo;
struct AccelerationStructure;
struct TLASInstance;
struct Viewport;
struct Scissor;
struct RenderingBeginInfo;
struct PipelineBarrier;
struct SubmitInfo;
struct PresentInfo;
struct DispatchRaysInfo;
struct GPUProperties;

using TextureHandle = Texture*;
using TextureViewHandle = TextureView*;
using BufferHandle = Buffer*;
using BufferViewHandle = BufferView*;

enum class API : uint32;
enum class GPUCapability : uint32;
enum class GPUPreference : uint32;
enum class GPUType : uint32;
enum class ValidationMode : uint32;
enum class ResourceFlags : uint32;
enum class LogicOp : uint32;
enum class ResourceLayout : uint32;
enum class ResourceUsage : uint32;
enum class MemoryUsage : uint32;
enum class Format : uint32;
enum class AddressMode : uint32;
enum class Filter : uint32;
enum class SampleCount : uint32;
enum class TextureDimension : uint32;
enum class TextureAspect : uint32;
enum class BorderColor : uint32;
enum class ShaderType : uint32;
enum class ShaderFormat : uint32;
enum class HLSLShaderModel : uint32;
enum class ColorSpace : uint32;
enum class ViewType : uint32;
enum class ComponentSwizzle : uint32;
enum class QueueType : uint32;
enum class TopologyType : uint32;
enum class PolygonMode : uint32;
enum class CullMode : uint32;
enum class FrontFace : uint32;
enum class BlendFactor : uint32;
enum class BlendOp : uint32;
enum class CompareOp : uint32;
enum class StencilOp : uint32;
enum class PipelineType : uint32;
enum class LoadOp : uint32;
enum class StoreOp : uint32;
enum class RenderingBeginInfoFlags : uint32;

}