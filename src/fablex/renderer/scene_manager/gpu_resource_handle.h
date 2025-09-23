#pragma once

#include "gpu_model.h"
#include "gpu_texture.h"
#include "gpu_material.h"
#include "core/utils.h"

#include <memory>
#include <variant>

namespace fe::renderer
{

class SceneManager;

using GPUResourceVariant = std::variant<
    GPUModel,
    GPUTexture,
    GPUMaterial
>;

using GPUResourceTypes = std::tuple<GPUModel, GPUTexture, GPUMaterial>;

template<typename TypeToCheck>
constexpr bool HasGPUResourceTypeV = TupleHasType<TypeToCheck, GPUResourceTypes>::value;

class GPUResourceHandle
{
public:
    GPUResourceHandle() = delete;

    template<typename T, typename... Params, typename = std::enable_if_t<HasGPUResourceTypeV<T>>>
    GPUResourceHandle(std::in_place_type_t<T>, Params&&... params)
        : m_resource(std::in_place_type<T>, std::forward<Params>(params)...)
    {}

    template<typename T, typename = std::enable_if_t<HasGPUResourceTypeV<T>>>
    T* get() const
    {
        return std::visit([](auto& obj) -> T* {
            using U = std::decay_t<decltype(obj)>;
            if constexpr (std::is_same_v<U, T>)
                return const_cast<T*>(&obj);
            else
                return nullptr;
        }, m_resource);
    }

    uint32 index() const
    {
        return std::visit([](auto& obj) { return obj.index(); }, m_resource);
    }

    UUID asset_uuid() const
    {
        return std::visit([](auto& obj) { return obj.asset_uuid(); }, m_resource);
    }

    bool upload_to_gpu(const SceneManager* sceneManager)
    {
        return std::visit([sceneManager](auto& obj){ return obj.upload_to_gpu(sceneManager); }, m_resource);
    }

    void reset()
    {
        return std::visit([](auto& obj) { return obj.reset(); }, m_resource);
    }

    template<typename Visitor>
    decltype(auto) visit(Visitor&& visitor)
    {
        return std::visit(std::forward<Visitor>(visitor), m_resource);
    }

    template<typename Visitor>
    decltype(auto) visit(Visitor&& visitor) const
    {
        return std::visit(std::forward<Visitor>(visitor), m_resource);
    }

private:
    GPUResourceVariant m_resource;
};

using GPUResourceHandlePtr = std::unique_ptr<GPUResourceHandle>;

template<typename T, typename... Params>
GPUResourceHandlePtr create_gpu_resource_handle(Params&&... params)
{
    return std::make_unique<GPUResourceHandle>(std::in_place_type<T>, std::forward<Params>(params)...);
}

}