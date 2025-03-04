#include "push_constants_manager.h"

namespace fe::renderer
{

PushConstants* PushConstantsManager::allocate_push_constants(const PushConstantsMetadata& pushConstantsMetadata)
{
    std::scoped_lock<std::mutex> locker(s_mutex);
    s_pushConstantsByName.emplace(pushConstantsMetadata.name, new PushConstants(pushConstantsMetadata));
    return s_pushConstantsByName[pushConstantsMetadata.name].get();
}

PushConstants* PushConstantsManager::get_push_constants(PushConstantsName name)
{
    std::scoped_lock<std::mutex> locker(s_mutex);
    auto it = s_pushConstantsByName.find(name);
    if (it == s_pushConstantsByName.end())
        return nullptr;
    return it->second.get();
}
    
}