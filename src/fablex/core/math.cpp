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
    return XMVectorGetX(XMVector3Length(XMVectorSubtract(vec1.data, vec2.data)));
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
    return XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(vec1.data, vec2.data)));
}

float distance_estimated(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector3LengthEst(XMVectorSubtract(vec1.data, vec2.data)));
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

bool Vector2::is_infinite(const Vector& vec)
{
    return XMVector2IsInfinite(vec.data);
}

bool Vector2::is_nan(const Vector& vec)
{
    return XMVector2IsNaN(vec.data);
}

bool Vector2::equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector2Equal(vec1.data, vec2.data);
}

bool Vector2::greater(const Vector& vec1, const Vector& vec2)
{
    return XMVector2Greater(vec1.data, vec2.data);
}

bool Vector2::greater_or_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector3GreaterOrEqual(vec1.data, vec2.data);
}

bool Vector2::less(const Vector& vec1, const Vector& vec2)
{
    return XMVector2Less(vec1.data, vec2.data);
}

bool Vector2::less_or_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector2LessOrEqual(vec1.data, vec2.data);
}

bool Vector2::near_equal(const Vector& vec1, const Vector& vec2, const Vector& epsilon)
{
    return XMVector2NearEqual(vec1.data, vec2.data, epsilon.data);
}

bool Vector2::not_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector2NotEqual(vec1.data, vec2.data);
}

float Vector2::angle_between_normals(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector2AngleBetweenNormals(vec1.data, vec2.data));
}

float Vector2::angle_between_normals_estimated(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector2AngleBetweenNormalsEst(vec1.data, vec2.data));
}

Vector Vector2::clamp_length(const Vector& vec, float lengthMin, float lengthMax)
{
    return XMVector2ClampLength(vec.data, lengthMin, lengthMax);
}

Vector Vector2::cross(const Vector& vec1, const Vector& vec2)
{
    return XMVector2Cross(vec1.data, vec2.data);
}

float Vector2::dot(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector2Dot(vec1.data, vec2.data));
}

float Vector2::length(const Vector& vec)
{
    return XMVectorGetX(XMVector2Length(vec.data));
}

float Vector2::length_estimated(const Vector& vec)
{
    return XMVectorGetX(XMVector2LengthEst(vec.data));
}

float Vector2::length_squared(const Vector& vec)
{
    return XMVectorGetX(XMVector2LengthSq(vec.data));
}

Vector Vector2::normalize(const Vector& vec)
{
    return XMVector2Normalize(vec.data);
}

Vector Vector2::normalize_estimated(const Vector& vec)
{
    return XMVector2NormalizeEst(vec.data);
}

Vector Vector2::transform(const Vector& vec, const Matrix& mat)
{
    return XMVector2Transform(vec.data, mat.data);
}

Vector Vector2::transform_coord(const Vector& vec, const Matrix& mat)
{
    return XMVector2TransformCoord(vec.data, mat.data);
}

Vector Vector2::transform_normal(const Vector& vec, const Matrix& mat)
{
    return XMVector2TransformNormal(vec.data, mat.data);
}

bool Vector3::is_infinite(const Vector& vec)
{
    return XMVector3IsInfinite(vec.data);
}

bool Vector3::is_nan(const Vector& vec)
{
    return XMVector3IsNaN(vec.data);
}

bool Vector3::equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector3Equal(vec1.data, vec2.data);
}

bool Vector3::greater(const Vector& vec1, const Vector& vec2)
{
    return XMVector3Greater(vec1.data, vec2.data);
}

bool Vector3::greater_or_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector3GreaterOrEqual(vec1.data, vec2.data);
}

bool Vector3::less(const Vector& vec1, const Vector& vec2)
{
    return XMVector3Less(vec1.data, vec2.data);
}

bool Vector3::less_or_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector3LessOrEqual(vec1.data, vec2.data);
}

bool Vector3::near_equal(const Vector& vec1, const Vector& vec2, const Vector& epsilon)
{
    return XMVector3NearEqual(vec1.data, vec2.data, epsilon.data);
}

bool Vector3::not_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector3NotEqual(vec1.data, vec2.data);
}

float Vector3::angle_between_normals(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector3AngleBetweenNormals(vec1.data, vec2.data));
}

float Vector3::angle_between_normals_estimated(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector3AngleBetweenNormalsEst(vec1.data, vec2.data));
}

Vector Vector3::clamp_length(const Vector& vec, float lengthMin, float lengthMax)
{
    return XMVector3ClampLength(vec.data, lengthMin, lengthMax);
}

Vector Vector3::cross(const Vector& vec1, const Vector& vec2)
{
    return XMVector3Cross(vec1.data, vec2.data);
}

float Vector3::dot(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector3Dot(vec1.data, vec2.data));
}

float Vector3::length(const Vector& vec)
{
    return XMVectorGetX(XMVector3Length(vec.data));
}

float Vector3::length_estimated(const Vector& vec)
{
    return XMVectorGetX(XMVector3LengthEst(vec.data));
}

float Vector3::length_squared(const Vector& vec)
{
    return XMVectorGetX(XMVector3LengthSq(vec.data));
}

Vector Vector3::normalize(const Vector& vec)
{
    return XMVector3Normalize(vec.data);
}

Vector Vector3::normalize_estimated(const Vector& vec)
{
    return XMVector3NormalizeEst(vec.data);
}

Vector Vector3::inverse_rotate(const Vector& vec, const Quat& quat)
{
    return XMVector3InverseRotate(vec.data, quat.data);
}

Vector Vector3::project(const Vector& vec, float viewportX, float viewportY, float viewportWidth, float viewportHeight, float viewportMinZ, float viewportMaxZ, const Matrix& projection, const Matrix& view, const Matrix& world)
{
    return XMVector3Project(vec.data, viewportX, viewportY, viewportWidth, viewportHeight, viewportMinZ, viewportMaxZ, projection.data, view.data, world.data);
}

Vector Vector3::unproject(const Vector& vec, float viewportX, float viewportY, float viewportWidth, float viewportHeight, float viewportMinZ, float viewportMaxZ, const Matrix& projection, const Matrix& view, const Matrix& world)
{
    return XMVector3Unproject(vec.data, viewportX, viewportY, viewportWidth, viewportHeight, viewportMinZ, viewportMaxZ, projection.data, view.data, world.data);
}

Vector Vector3::rotate(const Vector& vec, const Quat& quat)
{
    return XMVector3Rotate(vec.data, quat.data);
}

Vector Vector3::transform(const Vector& vec, const Matrix& mat)
{
    return XMVector3Transform(vec.data, mat.data);
}

Vector Vector3::transform_coord(const Vector& vec, const Matrix& mat)
{
    return XMVector3TransformCoord(vec.data, mat.data);
}

Vector Vector3::transform_normal(const Vector& vec, const Matrix& mat)
{
    return XMVector3TransformNormal(vec.data, mat.data);
}

bool Vector4::is_infinite(const Vector& vec)
{
    return XMVector4IsInfinite(vec.data);
}

bool Vector4::is_nan(const Vector& vec)
{
    return XMVector4IsNaN(vec.data);
}

bool Vector4::equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector4Equal(vec1.data, vec2.data);
}

bool Vector4::greater(const Vector& vec1, const Vector& vec2)
{
    return XMVector4Greater(vec1.data, vec2.data);
}

bool Vector4::greater_or_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector4GreaterOrEqual(vec1.data, vec2.data);
}

bool Vector4::less(const Vector& vec1, const Vector& vec2)
{
    return XMVector4Less(vec1.data, vec2.data);
}

bool Vector4::less_or_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector4LessOrEqual(vec1.data, vec2.data);
}

bool Vector4::near_equal(const Vector& vec1, const Vector& vec2, const Vector& epsilon)
{
    return XMVector4NearEqual(vec1.data, vec2.data, epsilon.data);
}

bool Vector4::not_equal(const Vector& vec1, const Vector& vec2)
{
    return XMVector4NotEqual(vec1.data, vec2.data);
}

float Vector4::angle_between_normals(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector4AngleBetweenNormals(vec1.data, vec2.data));
}

float Vector4::angle_between_normals_estimated(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector4AngleBetweenNormalsEst(vec1.data, vec2.data));
}

Vector Vector4::clamp_length(const Vector& vec, float lengthMin, float lengthMax)
{
    return XMVector4ClampLength(vec.data, lengthMin, lengthMax);
}

Vector Vector4::cross(const Vector& vec1, const Vector& vec2, const Vector& vec3)
{
    return XMVector4Cross(vec1.data, vec2.data, vec3.data);
}

float Vector4::dot(const Vector& vec1, const Vector& vec2)
{
    return XMVectorGetX(XMVector4Dot(vec1.data, vec2.data));
}

float Vector4::length(const Vector& vec)
{
    return XMVectorGetX(XMVector4Length(vec.data));
}

float Vector4::length_estimated(const Vector& vec)
{
    return XMVectorGetX(XMVector4LengthEst(vec.data));
}

float Vector4::length_squared(const Vector& vec)
{
    return XMVectorGetX(XMVector4LengthSq(vec.data));
}

Vector Vector4::normalize(const Vector& vec)
{
    return XMVector4Normalize(vec.data);
}

Vector Vector4::normalize_estimated(const Vector& vec)
{
    return XMVector4NormalizeEst(vec.data);
}

Vector Vector4::transform(const Vector& vec, const Matrix& mat)
{
    return XMVector4Transform(vec.data, mat.data);
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
    return XMMatrixMultiply(data, other.data);
}

Matrix Matrix::multiply_transpose(const Matrix& other) const
{
    return XMMatrixMultiplyTranspose(data, other.data);
}

Matrix Matrix::look_at_lh(const Vector& eyePos, const Vector& focusPos, const Vector& upDir)
{
    return XMMatrixLookAtLH(eyePos.data, focusPos.data, upDir.data);
}

Matrix Matrix::look_at_rh(const Vector& eyePos, const Vector& focusPos, const Vector& upDir)
{
    return XMMatrixLookAtRH(eyePos.data, focusPos.data, upDir.data);
}

Matrix Matrix::look_to_lh(const Vector& eyePos, const Vector& eyeDir, const Vector& upDir)
{
    return XMMatrixLookToLH(eyePos.data, eyeDir.data, upDir.data);
}

Matrix Matrix::look_to_rh(const Vector& eyePos, const Vector& eyeDir, const Vector& upDir)
{
    return XMMatrixLookToRH(eyePos.data, eyeDir.data, upDir.data);
}

Matrix Matrix::multiply(const Matrix& mat1, const Matrix& mat2)
{
    return XMMatrixMultiply(mat1.data, mat2.data);
}

Matrix Matrix::multiply_transpose(const Matrix& mat1, const Matrix& mat2)
{
    return XMMatrixMultiplyTranspose(mat1.data, mat2.data);
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
    return XMMatrixReflect(reflectionPlane.data);
}

Matrix Matrix::rotation(const Vector& axis, float angle, AngleUnit angleUnit)
{
    if (angleUnit == AngleUnit::DEGREES)
        angle = to_radians(angle);

    return XMMatrixRotationAxis(axis.data, angle);
}

Matrix Matrix::rotation(const Quat& quat)
{
    return XMMatrixRotationQuaternion(quat.data);
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

Matrix Matrix::create(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
    return XMMatrixSet(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33);
}

Matrix Matrix::translation(float offsetX, float offsetY, float offsetZ)
{
    return XMMatrixTranslation(offsetX, offsetY, offsetZ);
}

bool Quat::operator==(const Quat& other) const noexcept
{
    return XMQuaternionEqual(data, other.data);
}

bool Quat::operator!=(const Quat& other) const noexcept
{
    return XMQuaternionNotEqual(data, other.data);
}

Quat Quat::operator*(const Quat& other) const noexcept
{
    return XMQuaternionMultiply(data, other.data);
}

Quat& Quat::operator*=(const Quat& other) noexcept
{
    data = XMQuaternionMultiply(data, other.data);
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
    return XMVectorGetX(XMQuaternionDot(data, other.data));
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
    return XMVectorGetX(XMQuaternionDot(quat1.data, quat2.data));
}

Quat Quat::bary_centric(const Quat& quat1, const Quat& quat2, const Quat& quat3, float weightingFactor1, float weightingFactor2)
{
    return XMQuaternionBaryCentric(quat1.data, quat2.data, quat3.data, weightingFactor1, weightingFactor2);
}

Quat Quat::identity()
{
    return XMQuaternionIdentity();
}

Quat Quat::multiply(const Quat& quat1, const Quat& quat2)
{
    return XMQuaternionMultiply(quat1.data, quat2.data);
}

Quat Quat::rotation(const Matrix& rotationMatrix)
{
    return XMQuaternionRotationMatrix(rotationMatrix.data);
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

    return XMQuaternionRotationAxis(axis.data, angle);
}

Quat Quat::rotation_normal(const Vector& normalAxis, float angle, AngleUnit angleUnit)
{
    if (angleUnit == AngleUnit::DEGREES)
        angle = to_radians(angle);

    return XMQuaternionRotationNormal(normalAxis.data, angle);
}

Quat Quat::slerp(const Quat& quat1, const Quat& quat2, float controlFactor)
{
    return XMQuaternionSlerp(quat1.data, quat2.data, controlFactor);
}

Quat Quat::squad(const Quat& quat1, const Quat& quat2, const Quat& quat3, const Quat& quat4, float controlFactor)
{
    return XMQuaternionSquad(quat1.data, quat2.data, quat3.data, quat4.data, controlFactor);
}


}