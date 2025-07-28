#pragma once

#include "memory.h"
#include <utility>

namespace fe
{

template<typename T>
class DefaultAllocator
{
public:
    using ValueType = T;

    template<typename... Params>
    ValueType* allocate(Params&&... params)
    {
        return memory_new<ValueType>(std::forward<Params>(params)...);
    }

    void free(ValueType* ptr)
    {
        return memory_delete(ptr);
    }
};

}