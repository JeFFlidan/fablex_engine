#pragma once

#include "rhi/enums.h"
#include "core/math.h"
#include "core/primitives/aabb.h"

namespace fe::renderer
{

constexpr float MAX_8_BIT = 255.0f; // UNSIGNED
constexpr float MAX_S8_BIT = 127.5f; // SIGNED
constexpr float MAX_16_BIT = 65535.0f;

struct VertexPositionWind16Bit
{
    static constexpr rhi::Format FORMAT = rhi::Format::R16G16B16A16_UNORM;

    uint16 x = 0;
    uint16 y = 0;
    uint16 z = 0;
    uint16 w = 0;

    void from_full(const AABB& aabb, const Float3& position, uint8 wind)
    {
        Float3 lerpedPos = inverse_lerp(aabb.minPoint, aabb.maxPoint, position);
        x = static_cast<uint16>(lerpedPos.x * MAX_16_BIT);
        y = static_cast<uint16>(lerpedPos.y * MAX_16_BIT);
        z = static_cast<uint16>(lerpedPos.z * MAX_16_BIT);
        w = static_cast<uint16>(static_cast<float>(wind) / MAX_8_BIT * MAX_16_BIT);
    }

    Float3 get_position(const AABB& aabb) const
    {
        Float3 pos = Float3(
            static_cast<float>(x) / MAX_16_BIT,
            static_cast<float>(y) / MAX_16_BIT,
            static_cast<float>(z) / MAX_16_BIT
        );
        return lerp(aabb.minPoint, aabb.maxPoint, pos);
    }

    uint8 get_wind() const
    {
        return static_cast<uint8>(static_cast<float>(w) / MAX_16_BIT * MAX_8_BIT);
    }
};

struct VertexPositionWind32Bit
{
    static constexpr rhi::Format FORMAT = rhi::Format::R32G32B32A32_SFLOAT;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    void from_full(const Float3& position, uint8 wind)
    {
        x = position.x;
        y = position.y;
        z = position.z;
        w = wind / MAX_8_BIT;
    }

    Float3 get_position() const
    {
        return Float3(x, y, z);
    }

    uint8 get_wind() const
    {
        return static_cast<uint8>(w * MAX_8_BIT);
    }
};

struct VertexUV16Bit
{
    static constexpr rhi::Format FORMAT = rhi::Format::R16G16_UNORM;

    uint16 x = 0;
    uint16 y = 0;

    void from_full(const Float2& uv, const Float2& uvRangeMin = Float2(0.0f, 0.0f), const Float2& uvRangeMax = Float2(1.0f, 1.0f))
    {
        x = static_cast<uint16>(inverse_lerp(uvRangeMin.x, uvRangeMax.x, uv.x) * MAX_16_BIT);
        y = static_cast<uint16>(inverse_lerp(uvRangeMin.y, uvRangeMax.y, uv.y) * MAX_16_BIT);
    }
};

struct VertexUV32Bit
{
    static constexpr rhi::Format FORMAT = rhi::Format::R32G32_SFLOAT;

    float x = 0;
    float y = 0;

    void from_full(const Float2& uv, const Float2& uvRangeMin = Float2(0.0f, 0.0f), const Float2& uvRangeMax = Float2(1.0f, 1.0f))
    {
        x = inverse_lerp(uvRangeMin.x, uvRangeMax.x, uv.x);
        y = inverse_lerp(uvRangeMin.y, uvRangeMax.y, uv.y);
    }
};

struct VertexUVs16Bit
{
    static constexpr rhi::Format FORMAT = rhi::Format::R16G16B16A16_UNORM;

    VertexUV16Bit uv0;
    VertexUV16Bit uv1;
};

struct VertexUVs32Bit
{
    static constexpr rhi::Format FORMAT = rhi::Format::R32G32B32A32_SFLOAT;

    VertexUV32Bit uv0;
    VertexUV32Bit uv1;
};

struct VertexColor
{
    static constexpr rhi::Format FORMAT = rhi::Format::R8G8B8A8_UNORM;

    uint32 color = 0;
};

struct VertexNormal
{
    static constexpr rhi::Format FORMAT = rhi::Format::R8G8B8A8_SNORM;

    int8 x = 0;
    int8 y = 0;
    int8 z = 0;
    int8 w = 0;

    void from_full(const Float3& normal)
    {
        Float3 normalizedNormal(Vector3::normalize(normal));

        x = int8(normalizedNormal.x * MAX_S8_BIT);
        y = int8(normalizedNormal.y * MAX_S8_BIT);
        z = int8(normalizedNormal.z * MAX_S8_BIT);
        w = 0;
    }

    Float3 get_normal() const
    {
        return Float3(
            static_cast<float>(x) / MAX_S8_BIT,
            static_cast<float>(y) / MAX_S8_BIT,
            static_cast<float>(z) / MAX_S8_BIT
        );
    }
};

struct VertexTangent
{
    static constexpr rhi::Format FORMAT = rhi::Format::R8G8B8A8_SNORM;

    int8 x = 0;
    int8 y = 0;
    int8 z = 0;
    int8 w = 0;

    void from_full(const Float4& tangent)
    {
        Float4 normalizedTangent(Vector4::normalize(tangent));

        x = int8(normalizedTangent.x * MAX_S8_BIT);
        y = int8(normalizedTangent.y * MAX_S8_BIT);
        z = int8(normalizedTangent.z * MAX_S8_BIT);
        w = int8(normalizedTangent.w * MAX_S8_BIT);
    }

    Float4 get_tangent() const
    {
        return Float4(
            static_cast<float>(x) / MAX_S8_BIT,
            static_cast<float>(y) / MAX_S8_BIT,
            static_cast<float>(z) / MAX_S8_BIT,
            static_cast<float>(w) / MAX_S8_BIT
        );
    }
};

}