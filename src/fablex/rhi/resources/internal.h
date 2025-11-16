#pragma once

#include "enums.h"
#include "core/types.h"
#include "core/macro.h"
#include "core/window.h"
#include "core/flags_operations.h"

#ifdef WIN32
#define FE_VULKAN
#include <vulkan/vulkan.h>

#define FE_D3D12
#include <d3d12/d3d12.h>
#include <dxgi1_6.h>

#endif // WIN32

#include <string>
#include <vector>
#include <array>
#include <variant>

struct VmaAllocation_T;

namespace D3D12MA
{

class Allocation;

}

namespace fe::rhi
{

#define FE_VULKAN_RESOURCE() struct Vulkan

#define FE_DEFINE_RHI_RESOURCE()                                            \
private:                                                                    \
    std::variant<                                                           \
        std::monostate,                                                     \
        Vulkan,                                                             \
        D3D12                                                               \
    > m_apiData;                                                            \
public:                                                                     \
    Vulkan& vk() { return *std::get_if<Vulkan>(&m_apiData); }               \
    const Vulkan& vk() const { return *std::get_if<Vulkan>(&m_apiData); }   \
    const D3D12& d3d12() const { return *std::get_if<D3D12>(&m_apiData); }  \
    void init_vk() { m_apiData.emplace<Vulkan>(); }                         \
    void init_d3d12() { m_apiData.emplace<D3D12>(); }

}
