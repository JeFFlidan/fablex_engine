#pragma once

#include "core/pool_allocator.h"
#include <functional>

namespace fe::engine
{

class SparseSetEntry
{
public:
    SparseSetEntry()
    {
        m_id = s_counter++;
    }

    uint32 id() const { return m_id; }

private:
    inline static uint32 s_counter = 0;

    uint32 m_id = 0;
};

template<typename Component, uint32 PoolSize = 64>
class SparseSet
{
public:
    using ForEachHandler = std::function<void(Component*)>;

    SparseSet()
    {
        m_sparse.resize(1024, s_invalidIndex);
    }

    Component* insert(const SparseSetEntry& entry)
    {
        if (entry.id() >= m_sparse.size())
            m_sparse.resize(entry.id() + 1, s_invalidIndex);

        if (!has(entry))
        {
            m_sparse[entry.id()] = static_cast<uint32>(m_dense.size());
            m_dense.push_back(entry.id());
            m_components.push_back(m_allocator.allocate());
        }
        else
        {
            m_components.push_back(m_allocator.allocate());
        }

        return m_components.back();
    }

    void insert(const SparseSetEntry* entry)
    {
        insert(*entry);
    }

    void erase(const SparseSetEntry& entry)
    {
        if (!has(entry)) return;

        uint32_t index = m_sparse[entry.id()];
        uint32_t lastIndex = static_cast<uint32_t>(m_dense.size() - 1);
        uint32_t lastEntry = m_dense[lastIndex];

        std::swap(m_dense[index], m_dense[lastIndex]);
        std::swap(m_components[index], m_components[lastIndex]);

        m_sparse[lastEntry] = index;

        m_dense.pop_back();
        m_allocator.free(m_components.back());
        m_components.pop_back();
        m_sparse[entry.id()] = s_invalidIndex;
    }

    void erase(const SparseSetEntry* entry)
    {
        erase(*entry);
    }

    bool has(const SparseSetEntry& entry) const
    {
        return entry.id() < m_sparse.size()
            && m_sparse[entry.id()] < m_dense.size()
            && m_dense[m_sparse[entry.id()]] == entry.id();        
    }

    bool has(const SparseSetEntry* entry) const
    {
        has(*entry);
    }

    Component* get(const SparseSetEntry& entry) const
    {
        if (!has(entry))
            return nullptr;

        return m_components[m_sparse[entry.id()]];
    }

    Component* get(const SparseSetEntry* entry) const
    {
        return get(*entry);
    }

    void for_each(const ForEachHandler& handler)
    {
        for (Component* component : m_components)
            handler(component);
    }

    uint32 size() const { return m_dense.size(); }

private:
    static constexpr uint32 s_invalidIndex = ~0u;

    std::vector<uint32> m_sparse;
    std::vector<uint32> m_dense;
    std::vector<Component*> m_components;
    PoolAllocator<Component, PoolSize> m_allocator;
};

}