#pragma once

#include "enums.h"
#include "core/types.h"
#include "core/macro.h"
#include "core/window.h"
#include "core/flags_operations.h"

#ifdef WIN32
#define FE_VULKAN
#include <vulkan/vulkan.h>
#endif // WIN32

#include <string>
#include <vector>
#include <array>
#include <variant>

struct VmaAllocation_T;

namespace fe::rhi
{

#define FE_DEFINE_RHI_RESOURCE()    \
private:\
    std::variant<   \
        std::monostate, \
        Vulkan    \
    > m_apiData;\
public:\
    Vulkan& vk() { return *std::get_if<Vulkan>(&m_apiData); } \
    const Vulkan& vk() const { return *std::get_if<Vulkan>(&m_apiData); }   \
    void init_vk() { m_apiData.emplace<Vulkan>(); }

}
