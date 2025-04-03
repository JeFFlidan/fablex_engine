#pragma once

#include "types.h"
#include "platform/platform.h"
#include "lz4.h"
#include <string>
#include <functional>
#include <algorithm>

namespace fe
{

template<typename ...Ts>
struct Visitor : Ts ...
{
    Visitor(const Ts&... args) : Ts(args)... { }
    using Ts::operator()...;
};

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

    template<typename ...Ts>
    static auto make_visitor(Ts... lambdas)
    {
        return Visitor<Ts...>(lambdas...);
    }

    static std::string to_upper(const std::string& str)
    {
        std::string result = str;
        std::transform(str.begin(), str.end(), result.begin(),
            [](unsigned char c) { return std::toupper(c); });
        return result;
    }

    static std::string to_lower(const std::string& str)
    {
        std::string result = str;
        std::transform(str.begin(), str.end(), result.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    // Returns compressed data size
    static uint64 compress(
        const std::vector<uint8>& inData,
        std::vector<uint8>& outCompressedData
    )
    {
        uint64 compressStaging = LZ4_compressBound(inData.size());
        outCompressedData.resize(compressStaging);
        uint64 compressedDataSize = LZ4_compress_default(
            (const char*)inData.data(), 
            (char*)outCompressedData.data(), 
            inData.size(), 
            compressStaging
        );
        outCompressedData.resize(compressedDataSize);
        return compressedDataSize;
    }

    // outData array must be resized
    static void decompress(
        const std::vector<uint8>& inCompressedData,
        std::vector<uint8>& outData,
        uint64 compressedDataOffset = 0
    )
    {
        const uint8* compressedDataPtr = inCompressedData.data() + compressedDataOffset;
        uint64 compressedSize = inCompressedData.size() - compressedDataOffset;
        LZ4_decompress_safe(
            (const char*)compressedDataPtr, 
            (char*)outData.data(), 
            compressedSize, 
            outData.size()
        );
    }
};

template <typename T, typename Tuple>
struct TupleHasType;

template <typename T, typename... Us>
struct TupleHasType<T, std::tuple<Us...>> : std::disjunction<std::is_base_of<T, Us>...> {};

template <typename T, typename Tuple>
inline constexpr bool TupleHasTypeV = TupleHasType<T, Tuple>::value;

}