#pragma once

#include "rhi/fwd.h"
#include <variant>

namespace fe::rhi
{

using ResourceVariant = std::variant<
    Buffer*,
    BufferView*,
    Texture*,
    TextureView*,
    Shader*,
    Sampler*,
    Pipeline*,
    CommandPool*,
    CommandBuffer*,
    SwapChain*,
    Fence*,
    Semaphore*,
    AccelerationStructure*
>;

}