#pragma once

#include "gpu_resource_allocator.h"

#include "core/uuid.h"
#include "core/types.h"

#include <functional>

namespace fe::renderer
{

class SceneManager;
class GPUResourceBase;

using GPUResourceFreeHandler = std::function<void(GPUResourceBase*)>;

class GPUResourceBase
{
public:
    virtual ~GPUResourceBase() = default;

    virtual void build(SceneManager* sceneManager) = 0;
    virtual bool upload_to_gpu(const SceneManager* sceneManager) = 0;
    virtual GPUResourceFreeHandler free_handler() const = 0;

    virtual void reset()
    {
        m_indexInBuffer = s_invalidIndex;
        m_refCount = 0;
    }

    uint32 index() const { return m_indexInBuffer; }
    void index(uint32 newIndex) { m_indexInBuffer = newIndex; }

    bool is_valid() const
    {
        return m_indexInBuffer != s_invalidIndex;
    }

    uint32 ref_count() const { return m_refCount; }
    
    void increase_ref_count(uint32 value)
    {
        m_refCount += value;
    }

protected:
    static constexpr uint32 s_invalidIndex = ~0u;
    uint32 m_indexInBuffer = s_invalidIndex;
    uint32 m_refCount = 0;
};

template<typename Derived, typename AssetT>
class GPUResource : public GPUResourceBase
{
public:
    using AssetType = AssetT;

    GPUResource(AssetType* asset) : m_asset(asset) { }

    AssetType* asset() const { return m_asset; }
    UUID asset_uuid() const { return m_asset->get_uuid(); }

    virtual GPUResourceFreeHandler free_handler() const override
    {
        return [](GPUResourceBase* resource)
            {
                GPUResourceAllocator<GPUResource>::free(static_cast<Derived*>(resource));
            };
    }

protected:
    AssetType* m_asset = nullptr;
};

}