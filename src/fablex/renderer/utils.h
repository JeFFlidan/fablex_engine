#pragma once

#include "rhi/rhi.h"
#include "core/types.h"
#include "globals.h"
#include "fmt/core.h"
#include <string>

namespace fe::renderer
{

class Utils
{
public:
    static rhi::Buffer* create_uma_uniform_buffer(uint32 size);
    static rhi::Buffer* create_uma_storage_buffer(uint32 size = DEFAULT_GPU_BUFFER_SIZE);
    static rhi::Buffer* create_uma_buffer(uint32 size, rhi::ResourceUsage usage);

    static std::string create_per_frame_resource_name(std::string_view baseName);

    template<typename... Params>
    static std::string create_resource_name(std::string_view formatStr, Params&&... params)
    {
        return fmt::format(fmt::runtime(formatStr), params...);
    }

    template<typename... Params>
    static void set_debug_name(rhi::ResourceVariant resource, std::string_view formatStr, Params&&... params)
    {
        rhi::set_name(resource, create_resource_name(formatStr, params...));
    }

    // Debug name for per frame resource
    static void set_debug_name(rhi::ResourceVariant resource, const std::string& baseName);
};

}