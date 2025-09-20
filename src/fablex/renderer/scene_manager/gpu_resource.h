#pragma once

#include "core/types.h"

namespace fe::renderer
{

class GPUResource
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

}