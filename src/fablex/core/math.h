#pragma once

#include "macro.h"
#include "fmt/format.h"
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <cmath>

using namespace DirectX;
using namespace PackedVector;

namespace fe::math
{

struct Vector2
{
    XMVECTOR data;

    Vector2() = default;
    Vector2(const XMVECTOR& vec) : data(vec) { }
};

struct Vector3
{
    XMVECTOR data;

    Vector3() = default;
    Vector3(const XMVECTOR& vec) : data(vec) { }
};

struct Vector4
{
    XMVECTOR data;

    Vector4() = default;
    Vector4(const XMVECTOR& vec) : data(vec) { }
};

struct Matrix
{
    XMMATRIX data;

    Matrix() = default;
    Matrix(const XMMATRIX& mat) : data(mat) {}
};

struct Float2 : XMFLOAT2
{
    using XMFLOAT2::XMFLOAT2;

    Float2(const Vector2& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector2 to_vector() const noexcept
    {
        return XMLoadFloat2(this);
    }

    FORCE_INLINE void from_vector(const Vector2& vec) noexcept
    {
        XMStoreFloat2(this, vec.data);
    }

    float length() const
    {
        return std::sqrt(x * x + y * y);
    }
};

struct Float3 : XMFLOAT3
{
    using XMFLOAT3::XMFLOAT3;

    Float3(const Vector3& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector3 to_vector() const noexcept
    {
        return XMLoadFloat3(this);
    }

    FORCE_INLINE void from_vector(const Vector3& vec) noexcept
    {
        XMStoreFloat3(this, vec.data);
    }

    float length() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }
};

struct Float4 : XMFLOAT4
{
    using XMFLOAT4::XMFLOAT4;

    Float4(const Vector4& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector4 to_vector() const noexcept
    {
        return XMLoadFloat4(this);
    }

    FORCE_INLINE void from_vector(const Vector4& vec) noexcept
    {
        XMStoreFloat4(this, vec.data);
    }

    float length() const
    {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }
};

struct Int2 : XMINT2
{
    using XMINT2::XMINT2;

    Int2(const Vector2& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector2 to_vector() const noexcept
    {
        return XMLoadSInt2(this);
    }

    FORCE_INLINE void from_vector(const Vector2& vec) noexcept
    {
        XMStoreSInt2(this, vec.data);
    }
};

struct Int3 : XMINT3
{
    using XMINT3::XMINT3;

    Int3(const Vector3& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector3 to_vector() const noexcept
    {
        return XMLoadSInt3(this);
    }

    FORCE_INLINE void from_vector(const Vector3& vec) noexcept
    {
        XMStoreSInt3(this, vec.data);
    }
};

struct Int4 : XMINT4
{
    using XMINT4::XMINT4;

    Int4(const Vector4& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector4 to_vector() const noexcept
    {
        return XMLoadSInt4(this);
    }

    FORCE_INLINE void from_vector(const Vector4& vec) noexcept
    {
        XMStoreSInt4(this, vec.data);
    }
};

struct UInt2 : XMUINT2
{
    using XMUINT2::XMUINT2;

    UInt2(const Vector2& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector2 to_vector() const noexcept
    {
        return XMLoadUInt2(this);
    }

    FORCE_INLINE void from_vector(const Vector2& vec) noexcept
    {
        XMStoreUInt2(this, vec.data);
    }
};

struct UInt3 : XMUINT3
{
    using XMUINT3::XMUINT3;

    UInt3(const Vector3& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector3 to_vector() const noexcept
    {
        return XMLoadUInt3(this);
    }

    FORCE_INLINE void from_vector(const Vector3& vec) noexcept
    {
        XMStoreUInt3(this, vec.data);
    }
};

struct UInt4 : XMUINT4
{
    using XMUINT4::XMUINT4;

    UInt4(const Vector4& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector4 to_vector() const noexcept
    {
        return XMLoadUInt4(this);
    }

    FORCE_INLINE void from_vector(const Vector4& vec) noexcept
    {
        XMStoreUInt4(this, vec.data);
    }
};

struct Float3x3 : XMFLOAT3X3
{
    using XMFLOAT3X3::XMFLOAT3X3;

    Float3x3(const Matrix& mat) noexcept
    {
        from_matrix(mat);
    }

    FORCE_INLINE Matrix to_matrix() const noexcept
    {
        return Matrix(XMLoadFloat3x3(this));
    }

    FORCE_INLINE void from_matrix(const Matrix& mat) noexcept
    {
        XMStoreFloat3x3(this, mat.data);
    }
};

struct Float3x4 : XMFLOAT3X4
{
    using XMFLOAT3X4::XMFLOAT3X4;

    Float3x4(const Matrix& mat) noexcept
    {
        from_matrix(mat);
    }

    FORCE_INLINE Matrix to_matrix() const noexcept
    {
        return Matrix(XMLoadFloat3x4(this));
    }

    FORCE_INLINE void from_matrix(const Matrix& mat) noexcept
    {
        XMStoreFloat3x4(this, mat.data);
    }
};

struct Float4x4 : XMFLOAT4X4
{
    using XMFLOAT4X4::XMFLOAT4X4;

    Float4x4(const Matrix& mat) noexcept
    {
        from_matrix(mat);
    }

    FORCE_INLINE Matrix to_matrix() const noexcept
    {
        return Matrix(XMLoadFloat4x4(this));
    }

    FORCE_INLINE void from_matrix(const Matrix& mat) noexcept
    {
        XMStoreFloat4x4(this, mat.data);
    }
};

struct Float4x3 : XMFLOAT4X3
{
    using XMFLOAT4X3::XMFLOAT4X3;

    Float4x3(const Matrix& mat) noexcept
    {
        from_matrix(mat);
    }

    FORCE_INLINE Matrix to_matrix() const noexcept
    {
        return Matrix(XMLoadFloat4x3(this));
    }

    FORCE_INLINE void from_matrix(const Matrix& mat) noexcept
    {
        XMStoreFloat4x3(this, mat.data);
    }
};

inline float distance(const Float2& vec1, const Float2& vec2)
{
    return std::sqrt(
        (vec1.x - vec2.x) * (vec1.x - vec2.x) + 
        (vec1.y - vec2.y) * (vec1.y - vec2.y));
}

inline float distance(const Float3& vec1, const Float3& vec2)
{
    return std::sqrt(
        (vec1.x - vec2.x) * (vec1.x - vec2.x) + 
        (vec1.y - vec2.y) * (vec1.y - vec2.y) +
        (vec1.z - vec2.z) * (vec1.z - vec2.z));
}

inline float distance(const Float4& vec1, const Float4& vec2)
{
    return std::sqrt(
        (vec1.x - vec2.x) * (vec1.x - vec2.x) + 
        (vec1.y - vec2.y) * (vec1.y - vec2.y) +
        (vec1.z - vec2.z) * (vec1.z - vec2.z) +
        (vec1.w - vec2.w) * (vec1.w - vec2.w));
}

inline float distance(const Vector2& vec1, const Vector2& vec2)
{
    return XMVectorGetX(XMVector2Length(XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance(const Vector3& vec1, const Vector3& vec2)
{
    return XMVectorGetX(XMVector3Length(XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance(const Vector4& vec1, const Vector4& vec2)
{
    return XMVectorGetX(XMVector4Length(XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance_squared(const Float2& vec1, const Float2& vec2) 
{
    return (vec1.x - vec2.x) * (vec1.x - vec2.x) + 
        (vec1.y - vec2.y) * (vec1.y - vec2.y);
}

inline float distance_squared(const Float3& vec1, const Float3& vec2) 
{
    return (vec1.x - vec2.x) * (vec1.x - vec2.x) + 
        (vec1.y - vec2.y) * (vec1.y - vec2.y) +
        (vec1.z - vec2.z) * (vec1.z - vec2.z);
}

inline float distance_squared(const Float4& vec1, const Float4& vec2) 
{
    return (vec1.x - vec2.x) * (vec1.x - vec2.x) + 
        (vec1.y - vec2.y) * (vec1.y - vec2.y) +
        (vec1.z - vec2.z) * (vec1.z - vec2.z) +
        (vec1.w - vec2.w) * (vec1.w - vec2.w);
}

inline float distance_squared(const Vector2& vec1, const Vector2& vec2)
{
    return XMVectorGetX(XMVector2LengthSq(XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance_squared(const Vector3& vec1, const Vector3& vec2)
{
    return XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance_squared(const Vector4& vec1, const Vector4& vec2)
{
    return XMVectorGetX(XMVector4LengthSq(XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance_estimated(const Vector2& vec1, const Vector2& vec2)
{
    return XMVectorGetX(XMVector2LengthEst(XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance_estimated(const Vector3& vec1, const Vector3& vec2)
{
    return XMVectorGetX(XMVector3LengthEst(XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance_estimated(const Vector4& vec1, const Vector4& vec2)
{
    return XMVectorGetX(XMVector4LengthEst(XMVectorSubtract(vec1.data, vec2.data)));
}

inline float dot(const Float2& vec1, const Float2& vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y;
}

inline float dot(const Float3& vec1, const Float3& vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}

inline float dot(const Float4& vec1, const Float4& vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z + vec1.w * vec2.w;
}

}

template<>
struct fmt::formatter<fe::math::Float2>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::math::Float2& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {})", vec.x, vec.y);
    }
};

template<>
struct fmt::formatter<fe::math::Float3>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::math::Float3& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {}; {})", vec.x, vec.y, vec.z);
    }
};

template<>
struct fmt::formatter<fe::math::Float4>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::math::Float4& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {}; {}; {})", vec.x, vec.y, vec.z, vec.w);
    }
};


template<>
struct fmt::formatter<fe::math::Int2>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::math::Int2& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {})", vec.x, vec.y);
    }
};

template<>
struct fmt::formatter<fe::math::Int3>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::math::Int3& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {}; {})", vec.x, vec.y, vec.z);
    }
};

template<>
struct fmt::formatter<fe::math::Int4>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::math::Int4& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {}; {}; {})", vec.x, vec.y, vec.z, vec.w);
    }
};

template<>
struct fmt::formatter<fe::math::UInt2>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::math::UInt2& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {})", vec.x, vec.y);
    }
};

template<>
struct fmt::formatter<fe::math::UInt3>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::math::UInt3& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {}; {})", vec.x, vec.y, vec.z);
    }
};

template<>
struct fmt::formatter<fe::math::UInt4>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::math::UInt4& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {}; {}; {})", vec.x, vec.y, vec.z, vec.w);
    }
};
