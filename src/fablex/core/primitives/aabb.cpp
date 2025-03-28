#include "aabb.h"
#include "ray.h"
#include "sphere.h"

namespace fe
{

AABB::AABB(const Sphere& sphere)
{
    create(sphere);
}

AABB::AABB(const std::vector<Float3>& vertexPositions)
{
    create(vertexPositions);
}

AABB::AABB(const Float3& center, const Float3& halfWidth)
{
    create(center, halfWidth);
}

void AABB::create(const std::vector<Float3>& vertexPositions)
{
    minPoint = { FLOAT_MAX, FLOAT_MAX, FLOAT_MAX };
    maxPoint = { FLOAT_MIN, FLOAT_MIN, FLOAT_MIN };

    for (auto& pos : vertexPositions)
    {
        minPoint = min(minPoint, pos);
        maxPoint = max(maxPoint, pos);
    }
}

void AABB::create(const Float3& center, const Float3& halfWidth)
{
    minPoint = Float3(center.x - halfWidth.x, center.y - halfWidth.y, center.z - halfWidth.z);
    maxPoint = Float3(center.x + halfWidth.x, center.y + halfWidth.y, center.z + halfWidth.z);
}

void AABB::create(const Sphere& sphere)
{
    create(sphere.center, Float3(sphere.radius, sphere.radius, sphere.radius));
}

AABB AABB::transform(const Matrix& mat) const
{
    const Vector corners[8] = {
        Vector3::transform(minPoint, mat),
        Vector3::transform(Vector4::create(minPoint.x, maxPoint.y, minPoint.z, 1), mat),
        Vector3::transform(Vector4::create(minPoint.x, maxPoint.y, maxPoint.z, 1), mat),
        Vector3::transform(Vector4::create(minPoint.x, minPoint.z, maxPoint.z, 1), mat),
        Vector3::transform(Vector4::create(maxPoint.x, minPoint.y, maxPoint.z, 1), mat),
        Vector3::transform(Vector4::create(maxPoint.x, minPoint.y, minPoint.z, 1), mat),
        Vector3::transform(Vector4::create(maxPoint.x, maxPoint.y, minPoint.z, 1), mat),
        Vector3::transform(maxPoint, mat)
    };

    Vector min = corners[0];
    Vector max = corners[0];
    for (size_t i = 1; i != 8; ++i)
    {
        min = Vector::min(min, corners[i]);
        max = Vector::max(max, corners[i]);
    }

    AABB outputAABB;
    XMStoreFloat3(&outputAABB.minPoint, min);
    XMStoreFloat3(&outputAABB.maxPoint, max);
    return outputAABB;
}

AABB AABB::transform(const Float4x4& mat) const
{
    return transform(mat.to_matrix());
}

Matrix AABB::get_unorm_remap_matrix() const
{
    return Matrix::scaling(maxPoint.x - minPoint.x, maxPoint.y - minPoint.y, maxPoint.z - minPoint.z)
        * Matrix::translation(minPoint.x, minPoint.y, minPoint.z);
}

Float3 AABB::get_center() const
{
    return Float3((minPoint.x + maxPoint.x) * 0.5f, (minPoint.y + maxPoint.y) * 0.5f, (minPoint.z + maxPoint.z) * 0.5f);
}

Float3 AABB::get_half_width() const
{
    Float3 center = get_center();
    return Float3(abs(maxPoint.x - center.x), abs(maxPoint.y - center.y), abs(maxPoint.z - center.z));
}

float AABB::get_radius() const
{
    Float3 halfWidth = get_half_width();
    return std::sqrt(std::pow(halfWidth.x, 2.0f) + std::pow(halfWidth.y, 2.0f) + std::pow(halfWidth.z, 2.0f));
}

AABB::IntersectionType AABB::intersects(const AABB& aabb) const
{
    if (!is_valid() && !aabb.is_valid())
        return IntersectionType::OUTSIDE;
    
    const Float3& aMin = minPoint, &bMin = aabb.minPoint;
    const Float3& aMax = maxPoint, &bMax = aabb.maxPoint;

    if (aMin.x >= bMin.x && aMin.y >= bMin.y && aMin.z >= bMin.z
        && aMax.x <= bMax.x && aMax.y <= bMax.y && aMax.z <= bMax.z)
    {
        return IntersectionType::INSIDE;
    }

    if (aMax.x < bMin.x || aMin.x > bMax.x
        || aMax.y < bMin.y || aMin.y > bMax.y
        || aMax.z < bMax.z || aMin.z > aMax.z)
    {
        return IntersectionType::OUTSIDE;
    }

    return IntersectionType::INTERSECTS;
}

bool AABB::intersects(const Float3& point) const
{
    if (!is_valid())
        return false;
    if (point.x > maxPoint.x) return false;
    if (point.x < minPoint.x) return false;
    if (point.y > maxPoint.y) return false;
    if (point.y < minPoint.y) return false;
    if (point.z > maxPoint.z) return false;
    if (point.z < minPoint.z) return false;
    return true;
}

bool AABB::intersects(const Ray& ray) const
{
    if (!is_valid())
        return false;

    return true;
}

bool AABB::intersects(const Sphere& sphere) const
{
    return sphere.intersects(*this);
}

bool AABB::is_valid() const
{
    if (minPoint.x > maxPoint.x || minPoint.y > maxPoint.y || minPoint.z > maxPoint.z)
        return false;
    return true;
}

AABB AABB::merge(const AABB& a, const AABB& b)
{
    AABB aabb;
    aabb.minPoint = min(a.minPoint, b.minPoint);
    aabb.maxPoint = max(a.maxPoint, b.maxPoint);
    return aabb;
}


}
