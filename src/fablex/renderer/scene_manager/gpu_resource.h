#pragma once

#include "core/uuid.h"
#include "core/types.h"

namespace fe::renderer
{

class IndexedGPUObject
{
public:
    uint32 index() const { return m_indexInBuffer; }
    void index(uint32 newIndex) { m_indexInBuffer = newIndex; }

    bool is_valid() const
    {
        return m_indexInBuffer != s_invalidIndex;
    }

protected:
    static constexpr uint32 s_invalidIndex = ~0u;
    uint32 m_indexInBuffer = s_invalidIndex;
};

template<typename AssetType>
class GPUResource : public IndexedGPUObject
{
public:
    GPUResource(AssetType* asset) : m_asset(asset) { }

    AssetType* asset() const { return m_asset; }
    UUID asset_uuid() const { return m_asset->get_uuid(); }

protected:
    AssetType* m_asset = nullptr;
};

}