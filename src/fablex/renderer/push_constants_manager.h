#pragma once

#include "push_constants.h"
#include <mutex>
#include <memory>

namespace fe::renderer
{

class PushConstantsManager
{
public:
    static PushConstants* allocate_push_constants(const PushConstantsMetadata& pushConstantsMetadata);
    static PushConstants* get_push_constants(PushConstantsName name);

private:
    using PushConstantsMap = std::unordered_map<PushConstantsName, std::unique_ptr<PushConstants>>;

    inline static PushConstantsMap s_pushConstantsByName{};
    inline static std::mutex s_mutex{};
};

}