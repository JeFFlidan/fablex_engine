#include "device.h"
#include "renderer_config.h"
#include "rhi/utils.h"
#include "rhi/init_info.h"
#include "rhi/resources/swap_chain.h"

namespace fe::renderer
{

template<typename RawType, typename WrapperType>
RawType to_raw(WrapperType wrapper)
{
    return reinterpret_cast<RawType>(wrapper);
}

void Device::init(const RendererConfig* config)
{
    rhi::RHIInitInfo initInfo;
    initInfo.gpuPreference = rhi::GPUPreference::DISCRETE;
    initInfo.validationMode = config->get_validation_mode();

    rhi::fill_function_table(config->get_graphics_api());
    rhi::init(&initInfo);
}

void Device::cleanup()
{
    rhi::cleanup();
}

void Device::update_frame_index(SwapChainRef swapChain)
{
    if (g_frameIndex + 1 > swapChain->bufferCount - 1) 
        g_frameIndex = 0;
    else
        ++g_frameIndex;

    rhi::set_frame_index(g_frameIndex);
}

void Device::submit(const SubmitInfo& submitInfo)
{
    rhi::submit(&submitInfo.get());
}

void Device::present(const PresentInfo& presentInfo)
{
    rhi::present(&presentInfo.get());
}

void Device::wait_queue_idle(QueueType queueType)
{
    rhi::wait_queue_idle(queueType);
}

void Device::wait_for_fences(const HandleVector<FenceHandle>& fences)
{
    rhi::wait_for_fences(
        to_raw<rhi::Fence* const*>(fences.data()),
        fences.size()
    );
}

API Device::api()
{
    return rhi::get_api();
}

const rhi::GPUProperties& Device::gpu_properties()
{
    return rhi::get_gpu_properties();
}

GPUCapability Device::gpu_capability()
{
    return gpu_properties().capabilities;
}

uint64 Device::shader_identifier_size()
{
    return gpu_properties().shaderIdentifierSize;
}

uint64 Device::shader_identifier_alignment()
{
    return gpu_properties().shaderIdentifierAlignment;
}

uint64 Device::acceleration_structure_instance_size()
{
    return gpu_properties().accelerationStructureInstanceSize;
}

uint64 Device::timestamp_frequency()
{
    return gpu_properties().timestampFrequency;
}

uint64 Device::vendor_id()
{
    return gpu_properties().vendorID;
}

uint64 Device::device_id()
{
    return gpu_properties().deviceID;
}

const std::string& Device::gpu_name()
{
    return gpu_properties().gpuName;
}

const std::string& Device::drive_description()
{
    return gpu_properties().driverDescription;
}

bool Device::has_capability(GPUCapability capability)
{
    return has_flag(gpu_capability(), capability);
}

bool Device::is_validation_enabled()
{
    return gpu_properties().validationMode != rhi::ValidationMode::DISABLED;
}

uint32 Device::queue_count()
{
    return std::underlying_type_t<QueueType>(QueueType::COUNT);
}

}