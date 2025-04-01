#include "math.h"
#include <algorithm>

using namespace DirectX;
using namespace PackedVector;

namespace fe
{

float distance(const Float2& vec1, const Float2& vec2)
{
    return std::sqrt(
        (vec1.x - vec2.x) * (vec1.x - vec2.x) + 
        (vec1.y - vec2.y) * (vec1.y - vec2.y));
}

float distance(const Float3& vec1, const Float3& vec2)
{
    return std::sqrt(
        (vec1.x - vec2.x) * (vec1.x - vec2.x) + 
        (vec1.y - vec2.y) * (vec1.y - vec2.y) +
        (vec1.z - vec2.z) * (vec1.z - vec2.z));
}

float distance(const Float4& vec1, const Float4& vec2)
{
    return std::sqrt(
        (vec1.x - vec2.x) * (vec1.x - vec2.x) + 
        (vec1.y - vec2.y) * (vec1.y - vec2.y) +
        (vec1.z - vec2.z) * (vec1.z - vec2.z) +
        (vec1.w - vec2.w) * (vec1.w - vec2.w));
}

float distance(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector3Length(XMVectorSubtract(vec1, vec2)));
}

float distance_squared(const Float2& vec1, const Float2& vec2) 
{
    return (vec1.x - vec2.x) * (vec1.x - vec2.x) + 
        (vec1.y - vec2.y) * (vec1.y - vec2.y);
}

float distance_squared(const Float3& vec1, const Float3& vec2) 
{
    return (vec1.x - vec2.x) * (vec1.x - vec2.x) + 
        (vec1.y - vec2.y) * (vec1.y - vec2.y) +
        (vec1.z - vec2.z) * (vec1.z - vec2.z);
}

float distance_squared(const Float4& vec1, const Float4& vec2) 
{
    return (vec1.x - vec2.x) * (vec1.x - vec2.x) + 
        (vec1.y - vec2.y) * (vec1.y - vec2.y) +
        (vec1.z - vec2.z) * (vec1.z - vec2.z) +
        (vec1.w - vec2.w) * (vec1.w - vec2.w);
}

float distance_squared(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(vec1, vec2)));
}

float distance_estimated(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector3LengthEst(XMVectorSubtract(vec1, vec2)));
}

float dot(const Float2& vec1, const Float2& vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y;
}

float dot(const Float3& vec1, const Float3& vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}

float dot(const Float4& vec1, const Float4& vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z + vec1.w * vec2.w;
}

bool is_nearly_equal(const Float2& vec1, const Float2& vec2, float epsilon)
{
    return XMScalarNearEqual(vec1.x, vec2.x, epsilon)
        && XMScalarNearEqual(vec1.y, vec2.y, epsilon);
}

bool is_nearly_equal(const Float3& vec1, const Float3& vec2, float epsilon)
{
    return XMScalarNearEqual(vec1.x, vec2.x, epsilon)
        && XMScalarNearEqual(vec1.y, vec2.y, epsilon)
        && XMScalarNearEqual(vec1.z, vec2.z, epsilon);
}

bool is_nearly_equal(const Float4& vec1, const Float4& vec2, float epsilon)
{
    return XMScalarNearEqual(vec1.x, vec2.x, epsilon)
        && XMScalarNearEqual(vec1.y, vec2.y, epsilon)
        && XMScalarNearEqual(vec1.z, vec2.z, epsilon)
        && XMScalarNearEqual(vec1.w, vec2.w, epsilon);
}

// Rounds values like 19.9998, 1.9999. For now used only in quat_to_euler to cleanup degree values
float round(float value, float treshold)
{
    float intPart;
    float fracPart = std::modf(value, &intPart);

    if (std::abs(fracPart - 1.0f) < treshold)
        return std::ceil(value);

    return value;
}

float to_radians(float degrees)
{
    return XMConvertToRadians(degrees);
}

float to_degrees(float radians)
{
    return XMConvertToDegrees(radians);
}

half to_half(float value)
{
    return XMConvertFloatToHalf(value);
}

float to_float(half value)
{
    return XMConvertHalfToFloat(value);
}

Float3 to_euler(const Float4& quat, AngleUnit angleUnit)
{
    float pitch = 0;
    float yaw = 0;
    float roll = 0;

    // pitch
    {
        const float x = quat.w * quat.w - quat.x * quat.x - quat.y * quat.y + quat.z * quat.z;
        const float y = 2.0f * (quat.y * quat.z + quat.w * quat.x);

        if (is_nearly_equal(Float2(x, y), Float2(0, 0)))
            pitch = 2.0f * std::atan2(quat.x, quat.w);
        else
            pitch = std::atan2(y, x);
    }

    // yaw
    {
        yaw = std::asin(std::clamp(-2.0f * (quat.x * quat.z - quat.w * quat.y), -1.0f, 1.0f));
    }

    // roll
    {
        const float x = quat.w * quat.w + quat.x * quat.x - quat.y * quat.y - quat.z * quat.z;
        const float y = 2.0f * (quat.x * quat.y + quat.w * quat.z);

        if (is_nearly_equal(Float2(x, y), Float2(0, 0)))
            roll = 0;
        else
            roll = std::atan2(y, x);
    }

    switch (angleUnit)
    {
    case AngleUnit::DEGREES:
        return Float3(
            round(to_degrees(pitch)), 
            round(to_degrees(yaw)), 
            round(to_degrees(roll))
        );
    case AngleUnit::RADIANS:
        return Float3(pitch, yaw, roll);
    default:
        return Float3(0, 0, 0);
    }
}

Float3 to_euler(const Quat& quat, AngleUnit angleUnit)
{
    return to_euler(Float4(quat), angleUnit);
}

const Vector& Vector::operator+() noexcept
{
    return *this;
}

const Vector& Vector::operator-() noexcept
{
    data = XMVectorNegate(data);
    return *this;
}

Vector& Vector::operator+=(const Vector& other) noexcept
{
    data = XMVectorAdd(data, other);
    return *this;
}

Vector& Vector::operator-=(const Vector& other) noexcept
{
    data = XMVectorSubtract(data, other);
    return *this;
}

Vector& Vector::operator*=(const Vector& other) noexcept
{
    data = XMVectorMultiply(data, other);
    return *this;
}

Vector& Vector::operator/=(const Vector& other) noexcept
{
    data = XMVectorDivide(data, other);
    return *this;
}

Vector& Vector::operator*=(float value) noexcept
{
    data = XMVectorScale(data, value);
    return *this;
}

Vector& Vector::operator/=(float value) noexcept
{
    data = XMVectorScale(data, 1.0f / value);
    return *this;
}

Vector Vector::operator+(const Vector& other) const noexcept
{
    return XMVectorAdd(data, other);
}

Vector Vector::operator-(const Vector& other) const noexcept
{
    return XMVectorSubtract(data, other);
}

Vector Vector::operator*(const Vector& other) const noexcept
{
    return XMVectorMultiply(data, other);
}

Vector Vector::operator/(const Vector& other) const noexcept
{
    return XMVectorDivide(data, other);
}

Vector Vector::operator*(float value) const noexcept
{
    return XMVectorScale(data, value);
}

Vector Vector::operator/(float value) const noexcept
{
    return XMVectorScale(data, 1.0f / value);
}

float Vector::x() const noexcept
{
    return XMVectorGetX(data);
}

float Vector::y() const noexcept
{
    return XMVectorGetY(data);
}

float Vector::z() const noexcept
{
    return XMVectorGetZ(data);
}

float Vector::w() const noexcept
{
    return XMVectorGetW(data);
}

Vector Vector::add_angles(const Vector& other) const
{
    return XMVectorAddAngles(data, other);
}

Vector Vector::sub_angles(const Vector& other) const
{
    return XMVectorSubtractAngles(data, other);
}

float Vector::sum() const
{
    return XMVectorGetX(XMVectorSum(data));
}

Vector Vector::abs() const
{
    return XMVectorAbs(data);
}

Vector Vector::ceiling() const
{
    return XMVectorCeiling(data);
}

Vector Vector::floor() const
{
    return XMVectorFloor(data);
}

Vector Vector::round() const
{
    return XMVectorRound(data);
}

Vector Vector::truncate() const
{
    return XMVectorTruncate(data);
}

Vector Vector::saturate() const
{
    return XMVectorSaturate(data);
}

Vector Vector::clamp(float min, float max) const
{
    return XMVectorClamp(
        data, 
        Vector4::create(min, min, min, min), 
        Vector4::create(max, max, max, max)
    );
}

Vector Vector::pow(float value) const
{
    return XMVectorPow(
        data, 
        Vector4::create(value, value, value, value)
    );
}

Vector Vector::pow(const Vector& vec) const
{
    return XMVectorPow(data, vec);
}

Vector Vector::reciprocal() const
{
    return XMVectorReciprocal(data);
}

Vector Vector::reciprocal_estimated() const
{
    return XMVectorReciprocalEst(data);
}

Vector Vector::reciprocal_sqrt() const
{
    return XMVectorReciprocalSqrt(data);
}

Vector Vector::reciprocal_sqrt_estimated() const
{
    return XMVectorReciprocalSqrtEst(data);
}

Vector Vector::sqrt() const
{
    return XMVectorSqrt(data);
}

Vector Vector::sqrt_estimated() const
{
    return XMVectorSqrtEst(data);
}

Vector Vector::max(const Vector& vec1, const Vector& vec2)
{
    return XMVectorMax(vec1, vec2);
}

Vector Vector::min(const Vector& vec1, const Vector& vec2)
{
    return XMVectorMin(vec1, vec2);
}

Vector Vector::negative_multiply_subtract(const Vector& vec1, const Vector& vec2, const Vector& vec3)
{
    return XMVectorNegativeMultiplySubtract(vec1, vec2, vec3);
}

Vector Vector::multiply_add(const Vector& vec1, const Vector& vec2, const Vector& vec3)
{
    return XMVectorMultiplyAdd(vec1, vec2, vec3);
}

Vector operator*(float value, const Vector& vec)
{
    return vec * value;
}

bool Vector2::is_infinite(const Vector& vec)
{
    return XMVector2IsInfinite(vec);
}

bool Vector2::is_nan(const Vector& vec)
{
    return XMVector2IsNaN(vec);
}

bool Vector2::equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector2Equal(vec1, vec2);
}

bool Vector2::greater(const Vector& vec1, const Vector& vec2)
{
    return XMVector2Greater(vec1, vec2);
}

bool Vector2::greater_or_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector3GreaterOrEqual(vec1, vec2);
}

bool Vector2::less(const Vector& vec1, const Vector& vec2)
{
    return XMVector2Less(vec1, vec2);
}

bool Vector2::less_or_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector2LessOrEqual(vec1, vec2);
}

bool Vector2::near_equal(const Vector& vec1, const Vector& vec2, const Vector& epsilon)
{
    return XMVector2NearEqual(vec1, vec2, epsilon);
}

bool Vector2::not_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector2NotEqual(vec1, vec2);
}

float Vector2::angle_between_normals(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector2AngleBetweenNormals(vec1, vec2));
}

float Vector2::angle_between_normals_estimated(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector2AngleBetweenNormalsEst(vec1, vec2));
}

Vector Vector2::clamp_length(const Vector& vec, float lengthMin, float lengthMax)
{
    return XMVector2ClampLength(vec, lengthMin, lengthMax);
}

Vector Vector2::cross(const Vector& vec1, const Vector& vec2)
{
    return XMVector2Cross(vec1, vec2);
}

float Vector2::dot(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector2Dot(vec1, vec2));
}

float Vector2::length(const Vector& vec)
{
    return XMVectorGetX(XMVector2Length(vec));
}

float Vector2::length_estimated(const Vector& vec)
{
    return XMVectorGetX(XMVector2LengthEst(vec));
}

float Vector2::length_squared(const Vector& vec)
{
    return XMVectorGetX(XMVector2LengthSq(vec));
}

Vector Vector2::normalize(const Vector& vec)
{
    return XMVector2Normalize(vec);
}

Vector Vector2::normalize_estimated(const Vector& vec)
{
    return XMVector2NormalizeEst(vec);
}

Vector Vector2::transform(const Vector& vec, const Matrix& mat)
{
    return XMVector2Transform(vec, mat);
}

Vector Vector2::transform_coord(const Vector& vec, const Matrix& mat)
{
    return XMVector2TransformCoord(vec, mat);
}

Vector Vector2::transform_normal(const Vector& vec, const Matrix& mat)
{
    return XMVector2TransformNormal(vec, mat);
}

Vector Vector2::create(float val)
{
    return XMVectorSet(val, val, 0, 0);
}

Vector Vector2::create(float val1, float val2)
{
    return XMVectorSet(val1, val2, 0, 0);
}

bool Vector3::is_infinite(const Vector& vec)
{
    return XMVector3IsInfinite(vec);
}

bool Vector3::is_nan(const Vector& vec)
{
    return XMVector3IsNaN(vec);
}

bool Vector3::equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector3Equal(vec1, vec2);
}

bool Vector3::greater(const Vector& vec1, const Vector& vec2)
{
    return XMVector3Greater(vec1, vec2);
}

bool Vector3::greater_or_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector3GreaterOrEqual(vec1, vec2);
}

bool Vector3::less(const Vector& vec1, const Vector& vec2)
{
    return XMVector3Less(vec1, vec2);
}

bool Vector3::less_or_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector3LessOrEqual(vec1, vec2);
}

bool Vector3::near_equal(const Vector& vec1, const Vector& vec2, const Vector& epsilon)
{
    return XMVector3NearEqual(vec1, vec2, epsilon);
}

bool Vector3::not_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector3NotEqual(vec1, vec2);
}

float Vector3::angle_between_normals(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector3AngleBetweenNormals(vec1, vec2));
}

float Vector3::angle_between_normals_estimated(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector3AngleBetweenNormalsEst(vec1, vec2));
}

Vector Vector3::clamp_length(const Vector& vec, float lengthMin, float lengthMax)
{
    return XMVector3ClampLength(vec, lengthMin, lengthMax);
}

Vector Vector3::cross(const Vector& vec1, const Vector& vec2)
{
    return XMVector3Cross(vec1, vec2);
}

float Vector3::dot(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector3Dot(vec1, vec2));
}

float Vector3::length(const Vector& vec)
{
    return XMVectorGetX(XMVector3Length(vec));
}

float Vector3::length_estimated(const Vector& vec)
{
    return XMVectorGetX(XMVector3LengthEst(vec));
}

float Vector3::length_squared(const Vector& vec)
{
    return XMVectorGetX(XMVector3LengthSq(vec));
}

Vector Vector3::normalize(const Vector& vec)
{
    return XMVector3Normalize(vec);
}

Vector Vector3::normalize_estimated(const Vector& vec)
{
    return XMVector3NormalizeEst(vec);
}

Vector Vector3::inverse_rotate(const Vector& vec, const Quat& quat)
{
    return XMVector3InverseRotate(vec, quat);
}

Vector Vector3::project(const Vector& vec, float viewportX, float viewportY, float viewportWidth, float viewportHeight, float viewportMinZ, float viewportMaxZ, const Matrix& projection, const Matrix& view, const Matrix& world)
{
    return XMVector3Project(vec, viewportX, viewportY, viewportWidth, viewportHeight, viewportMinZ, viewportMaxZ, projection, view, world);
}

Vector Vector3::unproject(const Vector& vec, float viewportX, float viewportY, float viewportWidth, float viewportHeight, float viewportMinZ, float viewportMaxZ, const Matrix& projection, const Matrix& view, const Matrix& world)
{
    return XMVector3Unproject(vec, viewportX, viewportY, viewportWidth, viewportHeight, viewportMinZ, viewportMaxZ, projection, view, world);
}

Vector Vector3::rotate(const Vector& vec, const Quat& quat)
{
    return XMVector3Rotate(vec, quat);
}

Vector Vector3::transform(const Vector& vec, const Matrix& mat)
{
    return XMVector3Transform(vec, mat);
}

Vector Vector3::transform_coord(const Vector& vec, const Matrix& mat)
{
    return XMVector3TransformCoord(vec, mat);
}

Vector Vector3::transform_normal(const Vector& vec, const Matrix& mat)
{
    return XMVector3TransformNormal(vec, mat);
}

Vector Vector3::create(float val)
{
    return XMVectorSet(val, val, val, 0);
}

Vector Vector3::create(float val1, float val2, float val3)
{
    return XMVectorSet(val1, val2, val3, 0);
}

bool Vector4::is_infinite(const Vector& vec)
{
    return XMVector4IsInfinite(vec);
}

bool Vector4::is_nan(const Vector& vec)
{
    return XMVector4IsNaN(vec);
}

bool Vector4::equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector4Equal(vec1, vec2);
}

bool Vector4::greater(const Vector& vec1, const Vector& vec2)
{
    return XMVector4Greater(vec1, vec2);
}

bool Vector4::greater_or_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector4GreaterOrEqual(vec1, vec2);
}

bool Vector4::less(const Vector& vec1, const Vector& vec2)
{
    return XMVector4Less(vec1, vec2);
}

bool Vector4::less_or_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector4LessOrEqual(vec1, vec2);
}

bool Vector4::near_equal(const Vector& vec1, const Vector& vec2, const Vector& epsilon)
{
    return XMVector4NearEqual(vec1, vec2, epsilon);
}

bool Vector4::not_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector4NotEqual(vec1, vec2);
}

float Vector4::angle_between_normals(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector4AngleBetweenNormals(vec1, vec2));
}

float Vector4::angle_between_normals_estimated(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector4AngleBetweenNormalsEst(vec1, vec2));
}

Vector Vector4::clamp_length(const Vector& vec, float lengthMin, float lengthMax)
{
    return XMVector4ClampLength(vec, lengthMin, lengthMax);
}

Vector Vector4::cross(const Vector& vec1, const Vector& vec2, const Vector& vec3)
{
    return XMVector4Cross(vec1, vec2, vec3);
}

float Vector4::dot(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector4Dot(vec1, vec2));
}

float Vector4::length(const Vector& vec)
{
    return XMVectorGetX(XMVector4Length(vec));
}

float Vector4::length_estimated(const Vector& vec)
{
    return XMVectorGetX(XMVector4LengthEst(vec));
}

float Vector4::length_squared(const Vector& vec)
{
    return XMVectorGetX(XMVector4LengthSq(vec));
}

Vector Vector4::normalize(const Vector& vec)
{
    return XMVector4Normalize(vec);
}

Vector Vector4::normalize_estimated(const Vector& vec)
{
    return XMVector4NormalizeEst(vec);
}

Vector Vector4::transform(const Vector& vec, const Matrix& mat)
{
    return XMVector4Transform(vec, mat);
}

Vector Vector4::create(float val)
{
    return XMVectorSet(val, val, val, val);
}

Vector Vector4::create(float val1, float val2, float val3, float val4)
{
    return XMVectorSet(val1, val2, val3, val4);
}

bool Matrix::is_identity() const
{
    return XMMatrixIsIdentity(data);
}

bool Matrix::is_infinite() const
{
    return XMMatrixIsInfinite(data);
}

bool Matrix::is_nan() const
{
    return XMMatrixIsNaN(data);
}

void Matrix::decompose(Vector& outTrans, Vector& outScale, Quat& outQuat) const
{
    XMMatrixDecompose(&outScale.data, &outQuat.data, &outTrans.data, data);
}

Matrix Matrix::inverse(Vector* determinant) const
{
    return XMMatrixInverse(determinant ? &determinant->data : nullptr, data);
}

Matrix Matrix::transpose() const
{
    return XMMatrixTranspose(data);
}

Matrix Matrix::multiply(const Matrix& other) const
{
    return XMMatrixMultiply(data, other);
}

Matrix Matrix::multiply_transpose(const Matrix& other) const
{
    return XMMatrixMultiplyTranspose(data, other);
}

Matrix Matrix::look_at_lh(const Vector& eyePos, const Vector& focusPos, const Vector& upDir)
{
    return XMMatrixLookAtLH(eyePos, focusPos, upDir);
}

Matrix Matrix::look_at_rh(const Vector& eyePos, const Vector& focusPos, const Vector& upDir)
{
    return XMMatrixLookAtRH(eyePos, focusPos, upDir);
}

Matrix Matrix::look_to_lh(const Vector& eyePos, const Vector& eyeDir, const Vector& upDir)
{
    return XMMatrixLookToLH(eyePos, eyeDir, upDir);
}

Matrix Matrix::look_to_rh(const Vector& eyePos, const Vector& eyeDir, const Vector& upDir)
{
    return XMMatrixLookToRH(eyePos, eyeDir, upDir);
}

Matrix Matrix::multiply(const Matrix& mat1, const Matrix& mat2)
{
    return XMMatrixMultiply(mat1, mat2);
}

Matrix Matrix::multiply_transpose(const Matrix& mat1, const Matrix& mat2)
{
    return XMMatrixMultiplyTranspose(mat1, mat2);
}

Matrix Matrix::perspective_for_lh(float fovAngleY, float aspectRatio, float nearZ, float farZ)
{
    return XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ);
}

Matrix Matrix::perspective_for_rh(float fovAngleY, float aspectRatio, float nearZ, float farZ)
{
    return XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, nearZ, farZ);
}
Matrix Matrix::reflect(const Vector& reflectionPlane)
{
    return XMMatrixReflect(reflectionPlane);
}

Matrix Matrix::rotation(const Vector& axis, float angle, AngleUnit angleUnit)
{
    if (angleUnit == AngleUnit::DEGREES)
        angle = to_radians(angle);

    return XMMatrixRotationAxis(axis, angle);
}

Matrix Matrix::rotation(const Quat& quat)
{
    return XMMatrixRotationQuaternion(quat);
}

Matrix Matrix::rotation(float pitch, float yaw, float roll, AngleUnit angleUnit)
{
    if (angleUnit == AngleUnit::DEGREES)
    {
        pitch = to_radians(pitch);
        yaw = to_radians(yaw);
        roll = to_radians(roll);
    }

    return XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
}

Matrix Matrix::rotation_x(float angle, AngleUnit angleUnit)
{
    if (angleUnit == AngleUnit::DEGREES)
        angle = to_radians(angle);

    return XMMatrixRotationX(angle);
}

Matrix Matrix::rotation_y(float angle, AngleUnit angleUnit)
{
    if (angleUnit == AngleUnit::DEGREES)
        angle = to_radians(angle);

    return XMMatrixRotationY(angle);
}

Matrix Matrix::rotation_z(float angle, AngleUnit angleUnit)
{
    if (angleUnit == AngleUnit::DEGREES)
        angle = to_radians(angle);

    return XMMatrixRotationZ(angle);
}

Matrix Matrix::scaling(float scaleX, float scaleY, float scaleZ)
{
    return XMMatrixScaling(scaleX, scaleY, scaleZ);
}

Matrix Matrix::scaling(Float3 scale)
{
    return XMMatrixScaling(scale.x, scale.y, scale.z); 
}

Matrix Matrix::create(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
    return XMMatrixSet(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33);
}

Matrix Matrix::translation(float offsetX, float offsetY, float offsetZ)
{
    return XMMatrixTranslation(offsetX, offsetY, offsetZ);
}

Matrix Matrix::translation(Float3 offset)
{
    return XMMatrixTranslation(offset.x, offset.y, offset.z);
}

bool Quat::operator==(const Quat& other) const noexcept
{
    return XMQuaternionEqual(data, other);
}

bool Quat::operator!=(const Quat& other) const noexcept
{
    return XMQuaternionNotEqual(data, other);
}

Quat Quat::operator*(const Quat& other) const noexcept
{
    return XMQuaternionMultiply(data, other);
}

Quat& Quat::operator*=(const Quat& other) noexcept
{
    data = XMQuaternionMultiply(data, other);
    return *this;
}

bool Quat::is_identity() const
{
    return XMQuaternionIsIdentity(data);
}

bool Quat::is_infinite() const
{
    return XMQuaternionIsInfinite(data);
}

bool Quat::is_nan() const
{
    return XMQuaternionIsNaN(data);
}

float Quat::dot(const Quat& other) const
{
    return XMVectorGetX(XMQuaternionDot(data, other));
}

Quat Quat::inverse() const
{
    return XMQuaternionInverse(data);
}

Quat Quat::conjugate() const
{
    return XMQuaternionConjugate(data);
}

float Quat::length() const
{
    return XMVectorGetX(XMQuaternionLength(data));
}

float Quat::length_squared() const
{
    return XMVectorGetX(XMQuaternionLengthSq(data));
}

Quat Quat::ln() const
{
    return XMQuaternionLn(data);
}

Quat Quat::exp() const
{
    return XMQuaternionExp(data);
}

Quat Quat::normalize() const
{
    return XMQuaternionNormalize(data);
}

Quat Quat::normalize_estimated() const
{
    return XMQuaternionNormalizeEst(data);
}

Float3 Quat::to_euler(AngleUnit angleUnit) const
{
    return fe::to_euler(*this, angleUnit);
}

float Quat::dot(const Quat& quat1, const Quat& quat2)
{
    return XMVectorGetX(XMQuaternionDot(quat1, quat2));
}

Quat Quat::bary_centric(const Quat& quat1, const Quat& quat2, const Quat& quat3, float weightingFactor1, float weightingFactor2)
{
    return XMQuaternionBaryCentric(quat1, quat2, quat3, weightingFactor1, weightingFactor2);
}

Quat Quat::identity()
{
    return XMQuaternionIdentity();
}

Quat Quat::multiply(const Quat& quat1, const Quat& quat2)
{
    return XMQuaternionMultiply(quat1, quat2);
}

Quat Quat::rotation(const Matrix& rotationMatrix)
{
    return XMQuaternionRotationMatrix(rotationMatrix);
}

Quat Quat::rotation(float pitch, float yaw, float roll, AngleUnit angleUnit)
{
    if (angleUnit == AngleUnit::DEGREES)
    {
        pitch = to_radians(pitch);
        yaw = to_radians(yaw);
        roll = to_radians(roll);
    }

    return XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
}

Quat Quat::rotation_axis(const Vector& axis, float angle, AngleUnit angleUnit)
{
    if (angleUnit == AngleUnit::DEGREES)
        angle = to_radians(angle);

    return XMQuaternionRotationAxis(axis, angle);
}

Quat Quat::rotation_normal(const Vector& normalAxis, float angle, AngleUnit angleUnit)
{
    if (angleUnit == AngleUnit::DEGREES)
        angle = to_radians(angle);

    return XMQuaternionRotationNormal(normalAxis, angle);
}

Quat Quat::slerp(const Quat& quat1, const Quat& quat2, float controlFactor)
{
    return XMQuaternionSlerp(quat1, quat2, controlFactor);
}

Quat Quat::squad(const Quat& quat1, const Quat& quat2, const Quat& quat3, const Quat& quat4, float controlFactor)
{
    return XMQuaternionSquad(quat1, quat2, quat3, quat4, controlFactor);
}

Float2 min(const Float2& vec1, const Float2& vec2)
{
    return Float2(std::min(vec1.x, vec2.x), std::min(vec1.y, vec2.y));
}

Float3 min(const Float3& vec1, const Float3& vec2)
{
    return Float3(std::min(vec1.x, vec2.x), std::min(vec1.y, vec2.y), std::min(vec1.z, vec2.z));
}

Float4 min(const Float4& vec1, const Float4& vec2)
{
    return Float4(std::min(vec1.x, vec2.x), std::min(vec1.y, vec2.y), std::min(vec1.z, vec2.z), std::min(vec1.w, vec2.w));
}

Float2 max(const Float2& vec1, const Float2& vec2)
{
    return Float2(std::max(vec1.x, vec2.x), std::max(vec1.y, vec2.y));
}

Float3 max(const Float3& vec1, const Float3& vec2)
{
    return Float3(std::max(vec1.x, vec2.x), std::max(vec1.y, vec2.y), std::max(vec1.z, vec2.z));
}

Float4 max(const Float4& vec1, const Float4& vec2)
{
    return Float4(std::max(vec1.x, vec2.x), std::max(vec1.y, vec2.y), std::max(vec1.z, vec2.z), std::max(vec1.w, vec2.w));
}

float get_point_segment_distance(const Vector& point, const Vector& segmentA, const Vector& segmentB)
{
    float lengthSq = Vector3::length_squared(segmentB - segmentA);
    if (lengthSq == 0) return distance(point, segmentA);

    float t = std::max(0.0f, std::min(1.0f, Vector3::dot(point - segmentA, segmentB - segmentA) / lengthSq));
    Vector projection = segmentA + t * (segmentB - segmentA);
    return distance(point, projection);
}

Vector closest_point_on_line_segment(const Vector& point, const Vector& segmentA, const Vector& segmentB)
{
    Vector ab(segmentA - segmentB);
    float val = Vector3::dot(point - segmentA, ab) / Vector3::dot(ab, ab);
    return segmentA + Vector4::create(val).saturate() * ab;
}

float get_plane_point_distance(const Vector& point, const Vector& planeOrigin, const Vector& planeNormal)
{
    return Vector3::dot(planeNormal, point - planeOrigin);
}

Vector plane_from_point_normal(const Vector& point, const Vector& normal)
{
    return XMPlaneFromPointNormal(point, normal);
}

Vector plane_intersect_line(const Vector& point, const Vector& linePoint1, const Vector& linePoint2)
{
    return XMPlaneIntersectLine(point, linePoint1, linePoint2);
}

}