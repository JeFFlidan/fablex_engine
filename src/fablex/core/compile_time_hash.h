#pragma once

#include "types.h"

namespace fe
{

constexpr uint64 fnv_iterate(uint64 hash, uint8 c)
{
    return (hash * 0x100000001b3ull) ^ c;
}

template<size_t index>
constexpr uint64 compile_time_fnv1_inner(uint64 hash, const char* str)
{
    return compile_time_fnv1_inner<index - 1>(fnv_iterate(hash, uint8(str[index])), str);
}

template<>
constexpr uint64 compile_time_fnv1_inner<size_t(-1)>(uint64 hash, const char* str)
{
    return hash;
}

template<size_t len>
constexpr uint64 compile_time_fnv1(const char (&str)[len])
{
    return compile_time_fnv1_inner<len - 1>(0xcbf29ce484222325ull, str);
}

}