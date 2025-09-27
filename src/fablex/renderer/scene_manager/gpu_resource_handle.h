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

using GPUModelPtr = std::unique_ptr<GPUModel>;
using GPUTexturePtr = std::unique_ptr<GPUTexture>;
using GPUMaterialPtr = std::unique_ptr<GPUMaterial>;

using GPUResourceVariant = std::variant<
    GPUModelPtr,
    GPUTexturePtr,
    GPUMaterialPtr
>;

using GPUResourceTypes = std::tuple<GPUModel, GPUTexture, GPUMaterial>;

template<typename TypeToCheck>
constexpr bool HasGPUResourceTypeV = TupleHasType<TypeToCheck, GPUResourceTypes>::value;

class GPUResourceHandle
{
public:
    GPUResourceHandle() = delete;

    template<typename T, typename = std::enable_if_t<HasGPUResourceTypeV<T>>>
    GPUResourceHandle(T* resource)
        : m_resource(std::unique_ptr<T>(resource))
    {}

    template<typename T, typename = std::enable_if_t<HasGPUResourceTypeV<T>>>
    T* get() const
    {
        return std::visit([](auto& obj) -> T* {
            using UPtr = std::decay_t<decltype(obj)>;
            using U = typename UPtr::element_type; 
            if constexpr (std::is_same_v<U, T>)
                return const_cast<T*>(obj.get());
            else
                return nullptr;
        }, m_resource);
    }

    uint32 index() const
    {
        return std::visit([](auto& obj) { return obj->index(); }, m_resource);
    }

    UUID asset_uuid() const
    {
        return std::visit([](auto& obj) { return obj->asset_uuid(); }, m_resource);
    }

    bool upload_to_gpu(const SceneManager* sceneManager)
    {
        return std::visit([sceneManager](auto& obj){ return obj->upload_to_gpu(sceneManager); }, m_resource);
    }

    void reset()
    {
        return std::visit([](auto& obj) { return obj->reset(); }, m_resource);
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

}