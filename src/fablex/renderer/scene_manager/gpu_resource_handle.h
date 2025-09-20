#pragma once

#include "gpu_model.h"
#include "gpu_texture.h"
#include "gpu_material.h"
#include "core/utils.h"

#include <variant>

namespace fe::renderer
{

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
    T* get()
    {
        return std::visit([](auto& obj) -> T* {
            using U = std::decay_t<decltype(obj)>;
            if constexpr (std::is_same_v<U, T>)
                return &obj;
            else
                return nullptr;
        }, m_resource);
    }

    uint32 index() const
    {
        return std::visit([](auto& obj) { return obj.index(); }, m_resource);
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

template<typename T, typename = std::enable_if_t<HasGPUResourceTypeV<T>>>
class TGPUResourceHandle : public GPUResourceHandle
{
public:
    using Type = T;

    template<typename... Params>
    TGPUResourceHandle(Params&&... params) 
        : GPUResourceHandle(std::in_place_type<T>, std::forward<Params>(params)...)
    {

    }
};

using GPUModelHandle = TGPUResourceHandle<GPUModel>;
using GPUTextureHandle = TGPUResourceHandle<GPUTexture>;
using GPUMaterialHandle = TGPUResourceHandle<GPUMaterial>;

}