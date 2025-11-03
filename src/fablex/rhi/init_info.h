#pragma once

#include "resources/gpu_info.h"

namespace fe::rhi
{

struct RHIInitInfo
{
    GPUPreference gpuPreference{GPUPreference::DISCRETE};
    ValidationMode validationMode{ValidationMode::ENABLED};
};

}