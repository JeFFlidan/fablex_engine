#pragma once

#include "macro.h"
#include <DirectXMath.h>
#include <cmath>

namespace fe::math
{

struct Vector2
{
    DirectX::XMVECTOR data;

    Vector2() = default;
    Vector2(const DirectX::XMVECTOR& vec) : data(vec) { }

    // Calculates distance between 3D vectors
    FORCE_INLINE float distance(const Vector2& other)
    {
        return DirectX::XMVectorGetX(DirectX::XMVector2Length(DirectX::XMVectorSubtract(data, other.data)));
    }

    // Calculates squared distance between 3D vectors
    FORCE_INLINE float distance_squared(const Vector2& other)
    {
        return DirectX::XMVectorGetX(DirectX::XMVector2LengthSq(DirectX::XMVectorSubtract(data, other.data)));
    }
    
    FORCE_INLINE float distance_estimated(const Vector2& other)
    {
        return DirectX::XMVectorGetX(DirectX::XMVector2LengthEst(DirectX::XMVectorSubtract(data, other.data)));
    }
};

struct Vector3
{
    DirectX::XMVECTOR data;

    Vector3() = default;
    Vector3(const DirectX::XMVECTOR& vec) : data(vec) { }
};

struct Vector4
{
    DirectX::XMVECTOR data;

    Vector4() = default;
    Vector4(const DirectX::XMVECTOR& vec) : data(vec) { }
};

struct Matrix
{
    DirectX::XMMATRIX data;

    Matrix() = default;
    Matrix(const DirectX::XMMATRIX& mat) : data(mat) {}
};

struct Float2 : DirectX::XMFLOAT2
{
    using DirectX::XMFLOAT2::XMFLOAT2;

    Float2(const Vector2& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector2 to_vector() const noexcept
    {
        return DirectX::XMLoadFloat2(this);
    }

    FORCE_INLINE void from_vector(const Vector2& vec) noexcept
    {
        DirectX::XMStoreFloat2(this, vec.data);
    }

    float length() const
    {
        return std::sqrt(x * x + y * y);
    }
};

struct Float3 : DirectX::XMFLOAT3
{
    using DirectX::XMFLOAT3::XMFLOAT3;

    Float3(const Vector3& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector3 to_vector() const noexcept
    {
        return DirectX::XMLoadFloat3(this);
    }

    FORCE_INLINE void from_vector(const Vector3& vec) noexcept
    {
        DirectX::XMStoreFloat3(this, vec.data);
    }

    float length() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }
};

struct Float4 : DirectX::XMFLOAT4
{
    using DirectX::XMFLOAT4::XMFLOAT4;

    Float4(const Vector4& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector4 to_vector() const noexcept
    {
        return DirectX::XMLoadFloat4(this);
    }

    FORCE_INLINE void from_vector(const Vector4& vec) noexcept
    {
        DirectX::XMStoreFloat4(this, vec.data);
    }

    float length() const
    {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }
};

struct Int2 : DirectX::XMINT2
{
    using DirectX::XMINT2::XMINT2;

    Int2(const Vector2& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector2 to_vector() const noexcept
    {
        return DirectX::XMLoadSInt2(this);
    }

    FORCE_INLINE void from_vector(const Vector2& vec) noexcept
    {
        DirectX::XMStoreSInt2(this, vec.data);
    }
};

struct Int3 : DirectX::XMINT3
{
    using DirectX::XMINT3::XMINT3;

    Int3(const Vector3& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector3 to_vector() const noexcept
    {
        return DirectX::XMLoadSInt3(this);
    }

    FORCE_INLINE void from_vector(const Vector3& vec) noexcept
    {
        DirectX::XMStoreSInt3(this, vec.data);
    }
};

struct Int4 : DirectX::XMINT4
{
    using DirectX::XMINT4::XMINT4;

    Int4(const Vector4& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector4 to_vector() const noexcept
    {
        return DirectX::XMLoadSInt4(this);
    }

    FORCE_INLINE void from_vector(const Vector4& vec) noexcept
    {
        DirectX::XMStoreSInt4(this, vec.data);
    }
};

struct UInt2 : DirectX::XMUINT2
{
    using DirectX::XMUINT2::XMUINT2;

    UInt2(const Vector2& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector2 to_vector() const noexcept
    {
        return DirectX::XMLoadUInt2(this);
    }

    FORCE_INLINE void from_vector(const Vector2& vec) noexcept
    {
        DirectX::XMStoreUInt2(this, vec.data);
    }
};

struct UInt3 : DirectX::XMUINT3
{
    using DirectX::XMUINT3::XMUINT3;

    UInt3(const Vector3& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector3 to_vector() const noexcept
    {
        return DirectX::XMLoadUInt3(this);
    }

    FORCE_INLINE void from_vector(const Vector3& vec) noexcept
    {
        DirectX::XMStoreUInt3(this, vec.data);
    }
};

struct UInt4 : DirectX::XMUINT4
{
    using DirectX::XMUINT4::XMUINT4;

    UInt4(const Vector4& vec) noexcept
    {
        from_vector(vec);
    }

    FORCE_INLINE Vector4 to_vector() const noexcept
    {
        return DirectX::XMLoadUInt4(this);
    }

    FORCE_INLINE void from_vector(const Vector4& vec) noexcept
    {
        DirectX::XMStoreUInt4(this, vec.data);
    }
};

struct Float3x3 : DirectX::XMFLOAT3X3
{
    using DirectX::XMFLOAT3X3::XMFLOAT3X3;

    Float3x3(const Matrix& mat) noexcept
    {
        from_matrix(mat);
    }

    FORCE_INLINE Matrix to_matrix() const noexcept
    {
        return Matrix(DirectX::XMLoadFloat3x3(this));
    }

    FORCE_INLINE void from_matrix(const Matrix& mat) noexcept
    {
        DirectX::XMStoreFloat3x3(this, mat.data);
    }
};

struct Float3x4 : DirectX::XMFLOAT3X4
{
    using DirectX::XMFLOAT3X4::XMFLOAT3X4;

    Float3x4(const Matrix& mat) noexcept
    {
        from_matrix(mat);
    }

    FORCE_INLINE Matrix to_matrix() const noexcept
    {
        return Matrix(DirectX::XMLoadFloat3x4(this));
    }

    FORCE_INLINE void from_matrix(const Matrix& mat) noexcept
    {
        DirectX::XMStoreFloat3x4(this, mat.data);
    }
};

struct Float4x4 : DirectX::XMFLOAT4X4
{
    using DirectX::XMFLOAT4X4::XMFLOAT4X4;

    Float4x4(const Matrix& mat) noexcept
    {
        from_matrix(mat);
    }

    FORCE_INLINE Matrix to_matrix() const noexcept
    {
        return Matrix(DirectX::XMLoadFloat4x4(this));
    }

    FORCE_INLINE void from_matrix(const Matrix& mat) noexcept
    {
        DirectX::XMStoreFloat4x4(this, mat.data);
    }
};

struct Float4x3 : DirectX::XMFLOAT4X3
{
    using DirectX::XMFLOAT4X3::XMFLOAT4X3;

    Float4x3(const Matrix& mat) noexcept
    {
        from_matrix(mat);
    }

    FORCE_INLINE Matrix to_matrix() const noexcept
    {
        return Matrix(DirectX::XMLoadFloat4x3(this));
    }

    FORCE_INLINE void from_matrix(const Matrix& mat) noexcept
    {
        DirectX::XMStoreFloat4x3(this, mat.data);
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
    return DirectX::XMVectorGetX(DirectX::XMVector2Length(DirectX::XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance(const Vector3& vec1, const Vector3& vec2)
{
    return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance(const Vector4& vec1, const Vector4& vec2)
{
    return DirectX::XMVectorGetX(DirectX::XMVector4Length(DirectX::XMVectorSubtract(vec1.data, vec2.data)));
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
    return DirectX::XMVectorGetX(DirectX::XMVector2LengthSq(DirectX::XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance_squared(const Vector3& vec1, const Vector3& vec2)
{
    return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance_squared(const Vector4& vec1, const Vector4& vec2)
{
    return DirectX::XMVectorGetX(DirectX::XMVector4LengthSq(DirectX::XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance_estimated(const Vector2& vec1, const Vector2& vec2)
{
    return DirectX::XMVectorGetX(DirectX::XMVector2LengthEst(DirectX::XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance_estimated(const Vector3& vec1, const Vector3& vec2)
{
    return DirectX::XMVectorGetX(DirectX::XMVector3LengthEst(DirectX::XMVectorSubtract(vec1.data, vec2.data)));
}

inline float distance_estimated(const Vector4& vec1, const Vector4& vec2)
{
    return DirectX::XMVectorGetX(DirectX::XMVector4LengthEst(DirectX::XMVectorSubtract(vec1.data, vec2.data)));
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