#include "utils.h"

namespace fe::renderer
{

rhi::Buffer* Utils::create_uma_uniform_buffer(uint32 size)
{
    return create_uma_buffer(size, rhi::ResourceUsage::UNIFORM_BUFFER);
}

rhi::Buffer* Utils::create_uma_storage_buffer(uint32 size)
{
    return create_uma_buffer(size, rhi::ResourceUsage::STORAGE_BUFFER);
}

rhi::Buffer* Utils::create_uma_buffer(uint32 size, rhi::ResourceUsage usage)
{
    rhi::BufferInfo bufferInfo;
    bufferInfo.bufferUsage = usage;
    bufferInfo.memoryUsage = rhi::MemoryUsage::CPU_TO_GPU;
    bufferInfo.size = size;

    rhi::Buffer* buffer;
    rhi::create_buffer(&buffer, &bufferInfo);
    return buffer;
}

std::string Utils::create_per_frame_resource_name(std::string_view baseName)
{
    return create_resource_name("{}-{}", baseName, g_frameIndex);
}

void Utils::set_debug_name(rhi::ResourceVariant resource, const std::string& baseName)
{
    rhi::set_name(resource, create_per_frame_resource_name(baseName));
}

}