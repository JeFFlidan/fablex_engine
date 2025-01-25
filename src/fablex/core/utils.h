#pragma once

#include "types.h"
#include "platform/platform.h"
#include <string>
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

    static void convert_string(const std::wstring& from, std::string& to)
    {
#ifdef _WIN32
        int32_t length = WideCharToMultiByte(CP_UTF8, 0, from.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (length > 0)
        {
            to.resize(length - 1);
            WideCharToMultiByte(CP_UTF8, 0, from.c_str(), -1, to.data(), length, nullptr, nullptr);
        }
#endif
    }

    static void convert_string(const std::string& from, std::wstring& to)
    {
#ifdef _WIN32
        int32_t length = MultiByteToWideChar(CP_UTF8, 0, from.c_str(), -1, nullptr, 0);
        if (length > 0)
        {
            to.resize(length);
            MultiByteToWideChar(CP_UTF8, 0, from.c_str(), -1, to.data(), length);
        }
#endif
    }
};

}