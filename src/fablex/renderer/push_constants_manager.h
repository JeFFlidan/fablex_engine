#pragma once

#include "push_constants.h"
#include <mutex>
#include <memory>

namespace fe::renderer
{

class PushConstantsManager
{
public:
    PushConstants* allocate_push_constants(const PushConstantsMetadata& pushConstantsMetadata);
    PushConstants* get_push_constants(PushConstantsName name);

private:
    using PushConstantsMap = std::unordered_map<PushConstantsName, std::unique_ptr<PushConstants>>;

    PushConstantsMap s_pushConstantsByName;
    std::mutex s_mutex;
};

}