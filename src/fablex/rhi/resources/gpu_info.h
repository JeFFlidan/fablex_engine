#pragma once

#include "internal.h"

namespace fe::rhi
{

struct QueryPoolInfo
{
    // TODO
};

struct QueryPool
{
    // TODO
};

struct GPUMemoryUsage
{
    uint64_t total = 0;
    uint64_t usage = 0;
};

struct GPUProperties
{
    ValidationMode validationMode = ValidationMode::DISABLED;
    GPUCapability capabilities = GPUCapability::UNDEFINED;
    GPUType gpuType = GPUType::DISCRETE;
    uint64 shaderIdentifierSize = 0;
    uint64 shaderIdentifierAlignment = 0;
    uint64 accelerationStructureInstanceSize = 0;
    uint64 timestampFrequency = 0;
    uint32 vendorID = 0;
    uint32 deviceID = 0;
    std::string gpuName;
    std::string driverDescription;
};

}