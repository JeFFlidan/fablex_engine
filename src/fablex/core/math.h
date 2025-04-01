#pragma once

#include "macro.h"
#include "fmt/format.h"
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <cmath>

namespace fe
{

constexpr float FLOAT_MIN = std::numeric_limits<float>::min();
constexpr float FLOAT_MAX = std::numeric_limits<float>::max();
constexpr float PI = DirectX::XM_PI;
constexpr float EPSILON = std::numeric_limits<float>::epsilon();
inline DirectX::XMVECTOR EPSILON_VEC = DirectX::XMVectorSet(EPSILON, EPSILON, EPSILON, EPSILON);

enum class AngleUnit
{
    DEGREES,
    RADIANS
};

struct Vector;
struct Matrix;
struct Float2;
struct Float3;
struct Float4;
struct Quat;
struct Int2;
struct Int3;
struct Int4;
struct UInt2;
struct UInt3;
struct UInt4;

using half = DirectX::PackedVector::HALF;

float distance(const Float2& vec1, const Float2& vec2);
float distance(const Float3& vec1, const Float3& vec2);
float distance(const Float4& vec1, const Float4& vec2);
float distance(const Vector& vec1, const Vector& vec2);

float distance_squared(const Float2& vec1, const Float2& vec2);
float distance_squared(const Float3& vec1, const Float3& vec2);
float distance_squared(const Float4& vec1, const Float4& vec2);
float distance_squared(const Vector& vec1, const Vector& vec2);

float distance_estimated(const Vector& vec1, const Vector& vec2);

float dot(const Float2& vec1, const Float2& vec2);
float dot(const Float3& vec1, const Float3& vec2);
float dot(const Float4& vec1, const Float4& vec2);

bool is_nearly_equal(const Float2& vec1, const Float2& vec2, float epsilon = EPSILON);
bool is_nearly_equal(const Float3& vec1, const Float3& vec2, float epsilon = EPSILON);
bool is_nearly_equal(const Float4& vec1, const Float4& vec2, float epsilon = EPSILON);

float round(float value, float threshold = 1e-5);

float to_radians(float degrees);
float to_degrees(float radians);
half to_half(float value);
float to_float(half value);
Float3 to_euler(const Float4& quat, AngleUnit angleUnit = AngleUnit::DEGREES);
Float3 to_euler(const Quat& quat, AngleUnit angleUnit = AngleUnit::DEGREES);

struct Vector
{
    DirectX::XMVECTOR data;

    Vector() = default;
    Vector(const DirectX::XMVECTOR& vec) : data(vec) { }

    operator DirectX::XMVECTOR() const { return data; }

    const Vector& operator+() noexcept;
    const Vector& operator-() noexcept;

    Vector& operator+=(const Vector& other) noexcept;
    Vector& operator-=(const Vector& other) noexcept;
    Vector& operator*=(const Vector& other) noexcept;
    Vector& operator/=(const Vector& other) noexcept;
    Vector& operator*=(float value) noexcept;
    Vector& operator/=(float value) noexcept;

    Vector operator+(const Vector& other) const noexcept;
    Vector operator-(const Vector& other) const noexcept;
    Vector operator*(const Vector& other) const noexcept;
    Vector operator/(const Vector& other) const noexcept;
    Vector operator*(float value) const noexcept;
    Vector operator/(float value) const noexcept;

    float x() const noexcept;
    float y() const noexcept;
    float z() const noexcept;
    float w() const noexcept;

    /*!
     Two vectors must represent angles
    */
    Vector add_angles(const Vector& other) const;
    
    /* 
     Two vectors must represent angles
    */
    Vector sub_angles(const Vector& other) const;

    /*! 
     Computes the horizontal sum of the components of the Vector
    */
    float sum() const;

    Vector abs() const;
    Vector ceiling() const;
    Vector floor() const;
    Vector round() const;
    Vector truncate() const;
    Vector saturate() const;
    Vector clamp(float min, float max) const;
    Vector pow(float value) const;
    Vector pow(const Vector& vec) const;

    Vector reciprocal() const;
    Vector reciprocal_estimated() const;
    Vector reciprocal_sqrt() const;
    Vector reciprocal_sqrt_estimated() const;

    Vector sqrt() const;
    Vector sqrt_estimated() const;

    static Vector max(const Vector& vec1, const Vector& vec2);
    static Vector min(const Vector& vec1, const Vector& vec2);

    /*!
     Computes the difference of a third Vector and the product of the first two Vectors
     @param vec1 multiplier
     @param vec2 multiplicand
     @param vec3 subtrahend
    */
    static Vector negative_multiply_subtract(const Vector& vec1, const Vector& vec2, const Vector& vec3);

    /*!
     Computes the product of the first two Vectors added to the third Vector.
     @param vec1 multiplier
     @param vec2 multiplicand
     @param vec3 addend
    */
    static Vector multiply_add(const Vector& vec1, const Vector& vec2, const Vector& vec3);
};

Vector operator*(float value, const Vector& vec);

struct Vector2
{
    static bool is_infinite(const Vector& vec);
    static bool is_nan(const Vector& vec);

    static bool equal(const Vector& vec1, const Vector& vec2);
    static bool greater(const Vector& vec1, const Vector& vec2);
    static bool greater_or_equal(const Vector& vec1, const Vector& vec2);
    static bool less(const Vector& vec1, const Vector& vec2);
    static bool less_or_equal(const Vector& vec1, const Vector& vec2);
    static bool near_equal(const Vector& vec1, const Vector& vec2, const Vector& epsilon = EPSILON_VEC);
    static bool not_equal(const Vector& vec1, const Vector& vec2);

    static float angle_between_normals(const Vector& vec1, const Vector& vec2);
    static float angle_between_normals_estimated(const Vector& vec1, const Vector& vec2);
    static Vector clamp_length(const Vector& vec, float lengthMin, float lengthMax);
    static Vector cross(const Vector& vec1, const Vector& vec2);
    static float dot(const Vector& vec1, const Vector& vec2);
    static float length(const Vector& vec);
    static float length_estimated(const Vector& vec);
    static float length_squared(const Vector& vec);
    static Vector normalize(const Vector& vec);
    static Vector normalize_estimated(const Vector& vec);

    static Vector transform(const Vector& vec, const Matrix& mat);
    static Vector transform_coord(const Vector& vec, const Matrix& mat);
    static Vector transform_normal(const Vector& vec, const Matrix& mat);

    static Vector create(float val);
    static Vector create(float val1, float val2);
};

struct Vector3
{
    static bool is_infinite(const Vector& vec);
    static bool is_nan(const Vector& vec);

    static bool equal(const Vector& vec1, const Vector& vec2);
    static bool greater(const Vector& vec1, const Vector& vec2);
    static bool greater_or_equal(const Vector& vec1, const Vector& vec2);
    static bool less(const Vector& vec1, const Vector& vec2);
    static bool less_or_equal(const Vector& vec1, const Vector& vec2);
    static bool near_equal(const Vector& vec1, const Vector& vec2, const Vector& epsilon = EPSILON_VEC);
    static bool not_equal(const Vector& vec1, const Vector& vec2);

    static float angle_between_normals(const Vector& vec1, const Vector& vec2);
    static float angle_between_normals_estimated(const Vector& vec1, const Vector& vec2);
    static Vector clamp_length(const Vector& vec, float lengthMin, float lengthMax);
    static Vector cross(const Vector& vec1, const Vector& vec2);
    static float dot(const Vector& vec1, const Vector& vec2);
    static float length(const Vector& vec);
    static float length_estimated(const Vector& vec);
    static float length_squared(const Vector& vec);
    static Vector normalize(const Vector& vec);
    static Vector normalize_estimated(const Vector& vec);

    static Vector inverse_rotate(const Vector& vec, const Quat& quat);
    static Vector project(const Vector& vec, float viewportX, float viewportY, float viewportWidth, float viewportHeight, float viewportMinZ, float viewportMaxZ, const Matrix& projection, const Matrix& view, const Matrix& world);
    static Vector unproject(const Vector& vec, float viewportX, float viewportY, float viewportWidth, float viewportHeight, float viewportMinZ, float viewportMaxZ, const Matrix& projection, const Matrix& view, const Matrix& world);
    static Vector rotate(const Vector& vec, const Quat& quat);
    static Vector transform(const Vector& vec, const Matrix& mat);
    static Vector transform_coord(const Vector& vec, const Matrix& mat);
    static Vector transform_normal(const Vector& vec, const Matrix& mat);

    static Vector create(float val);
    static Vector create(float val1, float val2, float val3);
};

struct Vector4
{
    static bool is_infinite(const Vector& vec);
    static bool is_nan(const Vector& vec);

    static bool equal(const Vector& vec1, const Vector& vec2);
    static bool greater(const Vector& vec1, const Vector& vec2);
    static bool greater_or_equal(const Vector& vec1, const Vector& vec2);
    static bool less(const Vector& vec1, const Vector& vec2);
    static bool less_or_equal(const Vector& vec1, const Vector& vec2);
    static bool near_equal(const Vector& vec1, const Vector& vec2, const Vector& epsilon = EPSILON_VEC);
    static bool not_equal(const Vector& vec1, const Vector& vec2);

    static float angle_between_normals(const Vector& vec1, const Vector& vec2);
    static float angle_between_normals_estimated(const Vector& vec1, const Vector& vec2);
    static Vector clamp_length(const Vector& vec, float lengthMin, float lengthMax);
    static Vector cross(const Vector& vec1, const Vector& vec2, const Vector& vec3);
    static float dot(const Vector& vec1, const Vector& vec2);
    static float length(const Vector& vec);
    static float length_estimated(const Vector& vec);
    static float length_squared(const Vector& vec);
    static Vector normalize(const Vector& vec);
    static Vector normalize_estimated(const Vector& vec);

    static Vector transform(const Vector& vec, const Matrix& mat);

    static Vector create(float val);
    static Vector create(float val1, float val2, float val3, float val4);
};

struct Matrix
{
    DirectX::XMMATRIX data;

    Matrix() : data(DirectX::XMMatrixIdentity()) { }
    Matrix(const DirectX::XMMATRIX& mat) : data(mat) {}
    Matrix(const Vector& row1, const Vector& row2, const Vector& row3, const Vector& row4)
        : data(row1, row2, row3, row4) { }

    operator DirectX::XMMATRIX() const { return data; }

    const Matrix& operator+() const noexcept { return *this; }
    const Matrix& operator-() const noexcept { -data; return *this; }
    
    Matrix& operator+=(const Matrix& other) noexcept { data += other.data; return *this; }
    Matrix& operator-=(const Matrix& other) noexcept { data -= other.data; return *this; }
    Matrix& operator*=(const Matrix& other) noexcept { data *= other.data; return *this; }
    Matrix& operator*=(float value) noexcept { data *= value; return *this; }
    Matrix& operator/=(float value) noexcept { data /= value; return *this; }

    Matrix operator+(const Matrix& other) const noexcept { return data + other.data; }
    Matrix operator-(const Matrix& other) const noexcept { return data - other.data; }
    Matrix operator*(const Matrix& other) const noexcept { return data * other.data; }
    Matrix operator*(float value) const noexcept { return data * value; }
    Matrix operator/(float value) const noexcept { return data / value; }

    bool is_identity() const;
    bool is_infinite() const;
    bool is_nan() const;

    void decompose(Vector& outTrans, Vector& outScale, Quat& outQuat) const;
    Matrix inverse(Vector* determinant = nullptr) const;
    Matrix transpose() const;
    Matrix multiply(const Matrix& other) const;
    Matrix multiply_transpose(const Matrix& other) const;

    static Matrix identity() { return DirectX::XMMatrixIdentity(); }
    static Matrix look_at_lh(const Vector& eyePos, const Vector& focusPos, const Vector& upDir);
    static Matrix look_at_rh(const Vector& eyePos, const Vector& focusPos, const Vector& upDir);
    static Matrix look_to_lh(const Vector& eyePos, const Vector& eyeDir, const Vector& upDir);
    static Matrix look_to_rh(const Vector& eyePos, const Vector& eyeDir, const Vector& upDir);
    static Matrix multiply(const Matrix& mat1, const Matrix& mat2);
    static Matrix multiply_transpose(const Matrix& mat1, const Matrix& mat2);
    static Matrix perspective_for_lh(float fovAngleY, float aspectRatio, float nearZ, float farZ);
    static Matrix perspective_for_rh(float fovAngleY, float aspectRatio, float nearZ, float farZ);
    static Matrix reflect(const Vector& reflectionPlane);
    static Matrix rotation(const Vector& axis, float angle, AngleUnit angleUnit = AngleUnit::DEGREES);
    static Matrix rotation(const Quat& quat);
    static Matrix rotation(float pitch, float yaw, float roll, AngleUnit angleUnit = AngleUnit::DEGREES);
    static Matrix rotation_x(float angle, AngleUnit angleUnit = AngleUnit::DEGREES);
    static Matrix rotation_y(float angle, AngleUnit angleUnit = AngleUnit::DEGREES);
    static Matrix rotation_z(float angle, AngleUnit angleUnit = AngleUnit::DEGREES);
    static Matrix scaling(float scaleX, float scaleY, float scaleZ);
    static Matrix scaling(Float3 scale);
    static Matrix create(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);
    static Matrix translation(float offsetX, float offsetY, float offsetZ);
    static Matrix translation(Float3 offset);
};

struct Float2 : DirectX::XMFLOAT2
{
    using XMFLOAT2::XMFLOAT2;

    Float2(const Vector& vec) noexcept
    {
        from_vector(vec);
    }

    operator Vector() const { return to_vector(); }

    FORCE_INLINE Vector to_vector() const noexcept
    {
        return XMLoadFloat2(this);
    }

    FORCE_INLINE void from_vector(const Vector& vec) noexcept
    {
        XMStoreFloat2(this, vec);
    }

    float length() const
    {
        return std::sqrt(x * x + y * y);
    }
};

struct Float3 : DirectX::XMFLOAT3
{
    using XMFLOAT3::XMFLOAT3;

    Float3(const Vector& vec) noexcept
    {
        from_vector(vec);
    }

    operator Vector() const { return to_vector(); }

    FORCE_INLINE Vector to_vector() const noexcept
    {
        return XMLoadFloat3(this);
    }

    FORCE_INLINE void from_vector(const Vector& vec) noexcept
    {
        XMStoreFloat3(this, vec);
    }

    float length() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }
};

struct Quat
{
    DirectX::XMVECTOR data;

    Quat() : data(Quat::identity()) { }
    Quat(const DirectX::XMVECTOR& vec) noexcept : data(vec) { }
    Quat(const Vector& vec) noexcept : data(vec) {}

    operator DirectX::XMVECTOR() const { return data; }

    bool operator==(const Quat& other) const noexcept;
    bool operator!=(const Quat& other) const noexcept;
    Quat operator*(const Quat& other) const noexcept;
    Quat& operator*=(const Quat& other) noexcept;

    bool is_identity() const;
    bool is_infinite() const;
    bool is_nan() const;

    float dot(const Quat& other) const;
    Quat inverse() const;
    Quat conjugate() const;

    float length() const;
    float length_squared() const;

    Quat ln() const;
    Quat exp() const;
    
    Quat normalize() const;
    Quat normalize_estimated() const;

    Float3 to_euler(AngleUnit angleUnit = AngleUnit::DEGREES) const;

    static float dot(const Quat& quat1, const Quat& quat2);
    static Quat bary_centric(const Quat& quat1, const Quat& quat2, const Quat& quat3, float weightingFactor1, float weightingFactor2);
    static Quat identity();
    static Quat multiply(const Quat& quat1, const Quat& quat2);
    static Quat rotation(const Matrix& rotationMatrix);
    static Quat rotation(float pitch, float yaw, float roll, AngleUnit angleUnit = AngleUnit::DEGREES);
    static Quat rotation_axis(const Vector& axis, float angle, AngleUnit angleUnit = AngleUnit::DEGREES);
    static Quat rotation_normal(const Vector& normalAxis, float angle, AngleUnit angleUnit = AngleUnit::DEGREES);
    static Quat slerp(const Quat& quat1, const Quat& quat2, float controlFactor);
    static Quat squad(const Quat& quat1, const Quat& quat2, const Quat& quat3, const Quat& quat4, float controlFactor);
};

struct Float4 : DirectX::XMFLOAT4
{
    using XMFLOAT4::XMFLOAT4;

    Float4(const Vector& vec) noexcept
    {
        from_vector(vec);
    }

    Float4(const Quat& quat) noexcept
    {
        from_quat(quat);
    }

    operator Vector() const { return to_vector(); }

    FORCE_INLINE Vector to_vector() const noexcept
    {
        return XMLoadFloat4(this);
    }

    FORCE_INLINE Quat to_quat() const noexcept
    {
        return XMLoadFloat4(this);
    }

    FORCE_INLINE void from_vector(const Vector& vec) noexcept
    {
        XMStoreFloat4(this, vec);
    }

    FORCE_INLINE void from_quat(const Quat& quat) noexcept
    {
        XMStoreFloat4(this, quat);
    }

    float length() const
    {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }
};

struct Int2 : DirectX::XMINT2
{
    using XMINT2::XMINT2;

    Int2(const Vector& vec) noexcept
    {
        from_vector(vec);
    }

    operator Vector() const { return to_vector(); }

    FORCE_INLINE Vector to_vector() const noexcept
    {
        return XMLoadSInt2(this);
    }

    FORCE_INLINE void from_vector(const Vector& vec) noexcept
    {
        XMStoreSInt2(this, vec.data);
    }
};

struct Int3 : DirectX::XMINT3
{
    using XMINT3::XMINT3;

    Int3(const Vector& vec) noexcept
    {
        from_vector(vec);
    }

    operator Vector() const { return to_vector(); }

    FORCE_INLINE Vector to_vector() const noexcept
    {
        return XMLoadSInt3(this);
    }

    FORCE_INLINE void from_vector(const Vector& vec) noexcept
    {
        XMStoreSInt3(this, vec.data);
    }
};

struct Int4 : DirectX::XMINT4
{
    using XMINT4::XMINT4;

    Int4(const Vector& vec) noexcept
    {
        from_vector(vec);
    }

    operator Vector() const { return to_vector(); }

    FORCE_INLINE Vector to_vector() const noexcept
    {
        return XMLoadSInt4(this);
    }

    FORCE_INLINE void from_vector(const Vector& vec) noexcept
    {
        XMStoreSInt4(this, vec.data);
    }
};

struct UInt2 : DirectX::XMUINT2
{
    using XMUINT2::XMUINT2;

    UInt2(const Vector& vec) noexcept
    {
        from_vector(vec);
    }

    operator Vector() const { return to_vector(); }

    FORCE_INLINE Vector to_vector() const noexcept
    {
        return XMLoadUInt2(this);
    }

    FORCE_INLINE void from_vector(const Vector& vec) noexcept
    {
        XMStoreUInt2(this, vec.data);
    }
};

struct UInt3 : DirectX::XMUINT3
{
    using XMUINT3::XMUINT3;

    UInt3(const Vector& vec) noexcept
    {
        from_vector(vec);
    }

    operator Vector() const { return to_vector(); }

    FORCE_INLINE Vector to_vector() const noexcept
    {
        return XMLoadUInt3(this);
    }

    FORCE_INLINE void from_vector(const Vector& vec) noexcept
    {
        XMStoreUInt3(this, vec.data);
    }
};

struct UInt4 : DirectX::XMUINT4
{
    using XMUINT4::XMUINT4;

    UInt4(const Vector& vec) noexcept
    {
        from_vector(vec);
    }

    operator Vector() const { return to_vector(); }

    FORCE_INLINE Vector to_vector() const noexcept
    {
        return XMLoadUInt4(this);
    }

    FORCE_INLINE void from_vector(const Vector& vec) noexcept
    {
        XMStoreUInt4(this, vec.data);
    }
};

struct Float3x3 : DirectX::XMFLOAT3X3
{
    using XMFLOAT3X3::XMFLOAT3X3;

    Float3x3(const Matrix& mat) noexcept
    {
        from_matrix(mat);
    }

    operator Matrix() const { return to_matrix(); }

    FORCE_INLINE Matrix to_matrix() const noexcept
    {
        return Matrix(XMLoadFloat3x3(this));
    }

    FORCE_INLINE void from_matrix(const Matrix& mat) noexcept
    {
        XMStoreFloat3x3(this, mat.data);
    }
};

struct Float3x4 : DirectX::XMFLOAT3X4
{
    using XMFLOAT3X4::XMFLOAT3X4;

    Float3x4(const Matrix& mat) noexcept
    {
        from_matrix(mat);
    }

    operator Matrix() const { return to_matrix(); }

    FORCE_INLINE Matrix to_matrix() const noexcept
    {
        return Matrix(XMLoadFloat3x4(this));
    }

    FORCE_INLINE void from_matrix(const Matrix& mat) noexcept
    {
        XMStoreFloat3x4(this, mat.data);
    }
};

struct Float4x4 : DirectX::XMFLOAT4X4
{
    using XMFLOAT4X4::XMFLOAT4X4;

    Float4x4(const Matrix& mat) noexcept
    {
        from_matrix(mat);
    }

    operator Matrix() const { return to_matrix(); }

    FORCE_INLINE Matrix to_matrix() const noexcept
    {
        return Matrix(XMLoadFloat4x4(this));
    }

    FORCE_INLINE void from_matrix(const Matrix& mat) noexcept
    {
        XMStoreFloat4x4(this, mat.data);
    }
};

struct Float4x3 : DirectX::XMFLOAT4X3
{
    using XMFLOAT4X3::XMFLOAT4X3;

    Float4x3(const Matrix& mat) noexcept
    {
        from_matrix(mat);
    }

    operator Matrix() const { return to_matrix(); }

    FORCE_INLINE Matrix to_matrix() const noexcept
    {
        return Matrix(XMLoadFloat4x3(this));
    }

    FORCE_INLINE void from_matrix(const Matrix& mat) noexcept
    {
        XMStoreFloat4x3(this, mat.data);
    }
};

constexpr Float4x4 IDENTITY_MATRIX = Float4x4 {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

Float2 min(const Float2& vec1, const Float2& vec2);
Float3 min(const Float3& vec1, const Float3& vec2);
Float4 min(const Float4& vec1, const Float4& vec2);
Float2 max(const Float2& vec1, const Float2& vec2);
Float3 max(const Float3& vec1, const Float3& vec2);
Float4 max(const Float4& vec1, const Float4& vec2);

float get_point_segment_distance(const Vector& point, const Vector& segmentA, const Vector& segmentB);
Vector closest_point_on_line_segment(const Vector& point, const Vector& segmentA, const Vector& segmentB);
float get_plane_point_distance(const Vector& point, const Vector& planeOrigin, const Vector& planeNormal);

Vector plane_from_point_normal(const Vector& point, const Vector& normal);
Vector plane_intersect_line(const Vector& point, const Vector& linePoint1, const Vector& linePoint2);

}

template<>
struct fmt::formatter<fe::Float2>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::Float2& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {})", vec.x, vec.y);
    }
};

template<>
struct fmt::formatter<fe::Float3>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::Float3& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {}; {})", vec.x, vec.y, vec.z);
    }
};

template<>
struct fmt::formatter<fe::Float4>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::Float4& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {}; {}; {})", vec.x, vec.y, vec.z, vec.w);
    }
};


template<>
struct fmt::formatter<fe::Int2>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::Int2& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {})", vec.x, vec.y);
    }
};

template<>
struct fmt::formatter<fe::Int3>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::Int3& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {}; {})", vec.x, vec.y, vec.z);
    }
};

template<>
struct fmt::formatter<fe::Int4>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::Int4& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {}; {}; {})", vec.x, vec.y, vec.z, vec.w);
    }
};

template<>
struct fmt::formatter<fe::UInt2>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::UInt2& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {})", vec.x, vec.y);
    }
};

template<>
struct fmt::formatter<fe::UInt3>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::UInt3& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {}; {})", vec.x, vec.y, vec.z);
    }
};

template<>
struct fmt::formatter<fe::UInt4>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const fe::UInt4& vec, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}; {}; {}; {})", vec.x, vec.y, vec.z, vec.w);
    }
};
