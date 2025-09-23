#pragma once

#include "core/uuid.h"
#include "core/types.h"

namespace fe::renderer
{

template<typename AssetT>
class GPUResource
{
public:
    using AssetType = AssetT;

    GPUResource(AssetType* asset) : m_asset(asset) { }

    void reset()
    {
        m_indexInBuffer = s_invalidIndex;
        m_refCount = 0;
    }

    AssetType* asset() const { return m_asset; }
    UUID asset_uuid() const { return m_asset->get_uuid(); }

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

    AssetType* m_asset = nullptr;
};

}