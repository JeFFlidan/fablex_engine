#pragma once

#include "math.h"

namespace fe
{

struct Color8Bit
{
    uint32 rgba = 0;
    
    Color8Bit(uint32 rgba) : rgba(rgba) { }
    Color8Bit(uint8 r = 0, uint8 g = 0, uint8 b = 0, uint8 a = 255)
        : rgba((uint32)r | (uint32)g << 8 | uint32(b) << 16 | uint32(a) << 24) { }
    
    Color8Bit(const Float3& color) : Color8Bit(
        uint8(color.x * 255.0f),
        uint8(color.y * 255.0f),
        uint8(color.z * 255.0f)) { }
    
    Color8Bit(const Float4& color) : Color8Bit(
        uint8(color.x * 255.0f),
        uint8(color.y * 255.0f),
        uint8(color.z * 255.0f),
        uint8(color.w * 255.0f)) { }

    uint8 get_r() const { return rgba >> 0 & 0xFF; }
    uint8 get_g() const { return rgba >> 8 & 0xFF; }
    uint8 get_b() const { return rgba >> 16 & 0xFF; }
    uint8 get_a() const { return rgba >> 24 & 0xFF; }

    // Returns normalized data from red channel
    float get_nr() const { return static_cast<float>(get_r()) / 255.0f; }
    // Returns normalized data from green channel
    float get_ng() const { return static_cast<float>(get_g()) / 255.0f; }
    // Returns normalized data from blue channel
    float get_nb() const { return static_cast<float>(get_b()) / 255.0f; }
    // Returns normalized data from alpha channel
    float get_na() const { return static_cast<float>(get_a()) / 255.0f; }

    void set_r(uint8 r) { *this = { r, get_r(), get_b(), get_a() }; }
    void set_g(uint8 g) { *this = { get_r(), g, get_b(), get_a() }; }
    void set_b(uint8 b) { *this = { get_r(), get_g(), b, get_a()}; }
    void set_a(uint8 a) { *this = { get_r(), get_g(), get_b(), a }; }

    Float3 to_float3() const
    {
        return Float3{ get_nr(), get_ng(), get_nb() };
    }
    
    Float4 to_float4() const
    {
        return Float4{ get_nr(), get_ng(), get_nb(), get_na() };
    }

    operator Float3() const { return to_float3(); }
    operator Float4() const { return to_float4(); }
    operator uint32() const { return rgba; }
};

struct Color16Bit
{
    uint64 rgba = 0;
    
    Color16Bit(uint64 rgba) : rgba(rgba) { }
    Color16Bit(uint16 r = 0, uint16 g = 0, uint16 b = 0, uint16 a = 65535)
        : rgba((uint64)r | (uint64)g << 16 | (uint64)b << 32 | (uint64)a << 48) { }
    
    Color16Bit(const Float3& color) : Color16Bit(
        uint16(color.x * 65535.0f),
        uint16(color.y * 65535.0f),
        uint16(color.z * 65535.0f)) { }
    
    Color16Bit(const Float4& color) : Color16Bit(
        uint16(color.x * 65535.0f),
        uint16(color.y * 65535.0f),
        uint16(color.z * 65535.0f),
        uint16(color.w * 65535.0f)) { }

    uint16 get_r() const { return rgba >> 0 & 0xFFFF; }
    uint16 get_g() const { return rgba >> 16 & 0xFFFF; }
    uint16 get_b() const { return rgba >> 32 & 0xFFFF; }
    uint16 get_a() const { return rgba >> 48 & 0xFFFF; }

    float get_nr() const { return static_cast<float>(get_r()) / 65535.0f; }
    float get_ng() const { return static_cast<float>(get_g()) / 65535.0f; }
    float get_nb() const { return static_cast<float>(get_b()) / 65535.0f; }
    float get_na() const { return static_cast<float>(get_a()) / 65535.0f; }

    void set_r(uint16 r) { *this = { r, get_g(), get_b(), get_a() }; }
    void set_g(uint16 g) { *this = { get_r(), g, get_b(), get_a() }; }
    void set_b(uint16 b) { *this = { get_r(), get_g(), b, get_a() }; }
    void set_a(uint16 a) { *this = { get_r(), get_g(), get_b(), a }; }

    Float3 to_float3() const
    {
        return Float3{ get_nr(), get_ng(), get_nb() };
    }

    Float4 to_float4() const
    {
        return Float4{ get_nr(), get_ng(), get_nb(), get_na() };
    }

    operator Float3() const { return to_float3(); }
    operator Float4() const { return to_float4(); }
    operator uint64() const { return rgba; }
};

}