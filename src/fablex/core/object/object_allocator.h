#pragma once

#include <utility>

namespace fe
{

template<typename AllocatorType>
class ObjectAllocator
{
public:
    using ValueType = AllocatorType::ValueType;

    template<typename... Params>
    ValueType* allocate(Params&&... params)
    {
        return m_allocator.allocate(std::forward<Params>(params)...);
    }

    void free(ValueType* ptr)
    {
        m_allocator.free(ptr);
    }

private:
    AllocatorType m_allocator;
};

}