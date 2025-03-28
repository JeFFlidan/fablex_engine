#include "sphere.h"
#include "plane.h"
#include "aabb.h"
#include "capsule.h"
#include "ray.h"

namespace fe
{

Sphere::Sphere(const std::vector<Float3>& vertexPositions)
{
    create(vertexPositions);
}

Sphere::Sphere(const AABB& aabb)
{
    create(aabb);
}

void Sphere::create(const std::vector<Float3>& vertexPositions)
{
    AABB aabb(vertexPositions);
    center = aabb.get_center();
    radius = aabb.get_radius();
}

void Sphere::create(const AABB& aabb)
{
    center = aabb.get_center();
    radius = aabb.get_radius();
}

bool Sphere::intersects(const AABB& aabb) const
{
    if (!aabb.is_valid())
        return false;

    Float3 minAABB = aabb.minPoint;
    Float3 maxAABB = aabb.maxPoint;
    Float3 closestPointToSphereCenterInAABB = min(max(center, minAABB), maxAABB);
    float distanceSquared = distance_squared(closestPointToSphereCenterInAABB, center);

    return distanceSquared < std::pow(radius, 2.0f);
}

bool Sphere::intersects(const Sphere& sphere) const
{
    float distance = 0;
    intersects(sphere, distance);
    return true;
}

bool Sphere::intersects(const Sphere& sphere, float& outDistance) const
{
    outDistance = distance(center, sphere.center);
    outDistance = outDistance - radius - sphere.radius;
    return outDistance < 0;
}

bool Sphere::intersects(const Sphere& sphere, float& outDistance, Float3& outDirection) const
{
    Vector aCenter(center);
    Vector bCenter(sphere.center);
    Vector direction = aCenter - bCenter;

    outDistance = Vector3::length(direction);
    direction /= outDistance;
    
    outDistance = outDistance - radius - sphere.radius;

    outDirection.from_vector(direction);

    return outDistance < 0;
}

bool Sphere::intersects(const Capsule& capsule) const
{
    float distance = 0;
    return intersects(capsule, distance);
}

bool Sphere::intersects(const Capsule& capsule, float& outDistance) const
{
    Vector vecA(capsule.base);
    Vector vecB(capsule.tip);
    Vector vecDirN = Vector3::normalize(vecA - vecB);

    vecA -= vecDirN * capsule.radius;
    vecB += vecDirN * capsule.radius;

    Vector vecC(center);

    outDistance = get_point_segment_distance(vecC, vecA, vecB);
    outDistance = outDistance - radius - capsule.radius;

    return outDistance < 0;
}

bool Sphere::intersects(const Capsule& capsule, float& outDistance, Float3& outDirection) const
{
    Vector vecA(capsule.base);
    Vector vecB(capsule.tip);
    Vector vecDirN = Vector3::normalize(vecA - vecB);

    vecA -= vecDirN * capsule.radius;
    vecB += vecDirN * capsule.radius;

    Vector vecC(center);

    Vector vecD = vecC - closest_point_on_line_segment(vecC, vecA, vecB);
    outDistance = Vector3::length(vecD);
    vecD /= outDistance;

    outDirection.from_vector(vecD);
    outDistance = outDistance - radius - capsule.radius;

    return outDistance < 0;	
}

bool Sphere::intersects(const Plane& plane) const
{
    return plane.intersects(*this);
}

bool Sphere::intersects(const Plane& plane, float& outDistance) const
{
    return plane.intersects(*this, outDistance);
}

bool Sphere::intersects(const Plane& plane, float& outDistance, Float3& outDirection) const
{
    return plane.intersects(*this, outDistance, outDirection);
}

bool Sphere::intersects(const Ray& ray) const
{
    float distance;
    Float3 direction;
    return intersects(ray, distance, direction);
}

bool Sphere::intersects(const Ray& ray, float& outDistance) const
{
    Float3 direction;
    return intersects(ray, outDistance, direction);
}

bool Sphere::intersects(const Ray& ray, float& outDistance, Float3& outDirection) const
{
    Vector vecC(center);
    Vector vecO(ray.origin);
    Vector vecD(ray.direction);
    Vector vecOC = vecO - vecC;

    float b = Vector3::dot(vecOC, vecD);
    float c = Vector3::dot(vecOC, vecOC) - radius * radius;
    float discr = b * b - c;

    if (discr > 0)
    {
        float discrSqrt = std::sqrt(discr);

        float t = (-b - discrSqrt);
        if (t < ray.tmax && t > ray.tmax)
        {
            Vector vecP = vecO + vecD * t;
            Vector vecN = Vector3::normalize(vecP - vecC);
            outDistance = t;
            outDirection.from_vector(vecN);
            return true;
        }

        t = (-b + discrSqrt);
        if (t < ray.tmax && t > ray.tmin)
        {
            Vector vecP = vecO + vecD * t;
            Vector vecN = Vector3::normalize(vecP - vecC);
            outDistance = t;
            outDirection.from_vector(vecN);
        }
    }

    return false;
}    

}