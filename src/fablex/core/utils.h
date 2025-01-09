#pragma once

#include "types.h"
#include <functional>

namespace fe
{

class Utils
{
public:
    template<typename T>
    static void hash_combine(uint16& seed, const T& val)
    {
        seed ^= std::hash<T>()(val) + 0x9e37U + (seed << 3) + (seed >> 1);
    }

    template<typename T>
    static void hash_combine(uint32& seed, const T& val)
    {
        seed ^= std::hash<T>()(val) + 0x9e3779b9U + (seed << 6) + (seed >> 2);
    }

    template<typename T>
    static void hash_combine(uint64& seed, const T& val)
    {
        seed ^= std::hash<T>()(val) + 0x9e3779b97f4a7c15LLU  + (seed << 12) + (seed >> 4);
    }
};

}