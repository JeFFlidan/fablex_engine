#include "ray.h"
#include "aabb.h"
#include "sphere.h"
#include "capsule.h"
#include "plane.h"

namespace fe
{

Ray::Ray(
    const Float3& inOrigin,
    const Float3& inDirection,
    float inTMin,
    float inTMax
) : origin(inOrigin), direction(inDirection), tmin(inTMin), tmax(inTMax)
{
    directionInverse.from_vector(direction.to_vector().reciprocal());
}

Ray::Ray(const Vector& inOrigin, const Vector& inDirection, float inTMin, float inTMax)
    : origin(inOrigin), direction(inDirection), tmin(inTMin), tmax(inTMax)
{
    directionInverse.from_vector(inDirection.reciprocal());
}

bool Ray::intersects(const AABB& aabb) const
{
    return aabb.intersects(*this);
}

bool Ray::intersects(const Sphere& sphere) const
{
    return sphere.intersects(*this);
}

bool Ray::intersects(const Sphere& sphere, float& outDistance) const
{
    return sphere.intersects(*this, outDistance);
}

bool Ray::intersects(const Sphere& sphere, float& outDistance, Float3& outDirection) const
{
    return sphere.intersects(*this, outDistance, outDirection);
}

bool Ray::intersects(const Capsule& capsule) const
{
    return capsule.intersects(*this); 
}

bool Ray::intersects(const Capsule& capsule, float& outDistance) const
{
    return capsule.intersects(*this, outDistance);
}

bool Ray::intersects(const Capsule& capsule, float& outDistance, Float3& outDirection) const
{
    return capsule.intersects(*this, outDistance, outDirection);
}

bool Ray::intersects(const Plane& plane) const
{
    return plane.intersects(*this);
}

bool Ray::intersects(const Plane& plane, float& outDistance) const
{
    return plane.intersects(*this, outDistance);
}

bool Ray::intersects(const Plane& plane, float& outDistance, Float3& outDirection) const
{
    return plane.intersects(*this, outDistance, outDirection);
}

Float4x4 Ray::get_placement_orientation(const Float3& position, const Float3& normal) const
{
    Vector vecPosition(position);
    Vector vecNormal(normal);
    Vector vecOrigin(origin);
    Vector vecTangent = Vector3::normalize(Vector3::cross(vecNormal, vecPosition - vecOrigin));
    Vector vecBitangent = Vector3::normalize(Vector3::cross(vecTangent, vecNormal));

    return Matrix(vecTangent, vecNormal, vecBitangent, vecPosition);
}

Ray Ray::create_from_points(const Float3& a, const Float3& b)
{
    Vector vecA(a);
    Vector vecB(b);
    Vector vecDir = vecA - vecB;
    float dirLength = Vector3::length(vecDir);
    vecDir /= dirLength;

    return Ray(vecA, vecDir, 0, dirLength);
}

}