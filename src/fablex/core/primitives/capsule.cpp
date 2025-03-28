#include "capsule.h"
#include "aabb.h"
#include "sphere.h"
#include "ray.h"
#include "plane.h"

namespace fe
{

Capsule::Capsule(const AABB& aabb, float height)
{
    create(aabb, height);
}

Capsule::Capsule(const Sphere& sphere, float height)
{
    create(sphere, height);
}


Capsule::Capsule(const Float3& inBase, const Float3& inTip, float inRadius)
    : base(inBase), tip(inTip), radius(inRadius)
{
    
}

void Capsule::create(const AABB& aabb, float height)
{
    Float3 center = aabb.get_center();
    radius = aabb.get_radius();
    FE_CHECK(radius >= 0);

    base = { center.x, center.y - radius, center.z };
    tip = { base.x, base.y + height, base.z };
}

void Capsule::create(const Sphere& sphere, float height)
{
    Float3 center = sphere.center;
    radius = sphere.radius;
    FE_CHECK(radius >= 0);
    base = { center.x, center.y - radius, center.z };
    tip = { base.x, base.y + height, base.z };
}

void Capsule::create(const Float3& inBase, const Float3& inTip, float inRadius)
{
    base = inBase;
    tip = inTip;
    radius = inRadius;
}

// https://wickedengine.net/2020/04/26/capsule-collision-detection/
bool Capsule::intersects(const Capsule& other, Float3& outPosition, Float3& outIncidentNormal, float& outPenetrationDepth) const
{
    if (get_aabb().intersects(other.get_aabb()) == AABB::IntersectionType::OUTSIDE)
        return false;

    Vector aBase(base);
    Vector aTip(tip);
    Vector aNormal = Vector3::normalize(aTip - aBase);
    Vector aLineEndOffset = aNormal * radius;
    Vector aA = aBase + aLineEndOffset;
    Vector aB = aTip - aLineEndOffset;

    Vector bBase(other.base);
    Vector bTip(other.tip);
    Vector bNormal = Vector3::normalize(bTip - bBase);
    Vector bLineEndOffset = bNormal * other.radius;
    Vector bA = bBase + bLineEndOffset;
    Vector bB = bTip - bLineEndOffset;

    Vector vec0 = bA - aA;
    Vector vec1 = bB - aA;
    Vector vec2 = bA - aB;
    Vector vec3 = bB - aB;

    float squaredDistance0 = Vector3::length_squared(vec0);
    float squaredDistance1 = Vector3::length_squared(vec1);
    float squaredDistance2 = Vector3::length_squared(vec2);
    float squaredDistance3 = Vector3::length_squared(vec3);

    Vector bestA;
    if (squaredDistance2 < squaredDistance0 || squaredDistance2 < squaredDistance1
        || squaredDistance3 < squaredDistance0 || squaredDistance3 < squaredDistance1)
    {
        bestA = aB;
    }
    else
    {
        bestA = aA;
    }

    Vector bestB = closest_point_on_line_segment(bA, bB, bestA);
    bestA = closest_point_on_line_segment(aA, aB, bestB);

    Vector normal = bestA - bestB;
    float length = Vector3::length(normal);
    normal /= length;

    outPosition.from_vector(bestA - normal * radius);
    outIncidentNormal.from_vector(normal);
    outPenetrationDepth = radius + other.radius - length;
    
    return outPenetrationDepth > 0;
}

bool Capsule::intersects(const Sphere& sphere) const
{
    return sphere.intersects(*this);
}

bool Capsule::intersects(const Sphere& sphere, float& outDistance) const
{
    bool intersects = sphere.intersects(*this, outDistance);
    outDistance = -outDistance;
    return intersects;
}

bool Capsule::intersects(const Sphere& sphere, float& outDistance, Float3& outDirection) const
{
    bool intersects = sphere.intersects(*this, outDistance);
    outDistance = -outDistance;
    outDirection.x *= -1;
    outDirection.y *= -1;
    outDirection.z *= -1;
    return intersects;
}

bool Capsule::intersects(const Plane& plane) const
{
    return plane.intersects(*this);
}

bool Capsule::intersects(const Plane& plane, float& outDistance) const
{
    bool intersects = plane.intersects(*this, outDistance);
    outDistance = -outDistance;
    return intersects;
}

bool Capsule::intersects(const Plane& plane, float& outDistance, Float3& outDirection) const
{
    bool intersects = plane.intersects(*this, outDistance, outDirection);
    outDistance = -outDistance;
    return intersects;
}

bool Capsule::intersects(const Ray& ray) const
{
    float outDistance;
    Float3 outDirection;
    return intersects(ray, outDistance, outDirection);
}

bool Capsule::intersects(const Ray& ray, float& outDistance) const
{
    Float3 outDirection;
    return intersects(ray, outDistance, outDirection);
}

bool Capsule::intersects(const Ray& ray, float& outDistance, Float3& outDirection) const
{
    Vector A(base);
    Vector B(tip);
    Vector L = Vector3::normalize(A - B);

    A -= L * radius;
    B += L * radius;

    Vector O(ray.origin);
    Vector D(ray.direction);
    Vector C = Vector3::normalize(Vector3::cross(L, A - O));
    Vector N = Vector3::cross(L, C);
    Vector plane = plane_from_point_normal(A, N);
    Vector I = plane_intersect_line(plane, O, O + D * ray.tmax);
    Vector P = closest_point_on_line_segment(I, A, B);

    Sphere sphere;
    sphere.radius = radius;
    sphere.center = P;

    return sphere.intersects(ray, outDistance, outDirection);
}

AABB Capsule::get_aabb() const
{
    AABB baseAABB(base, Float3(radius, radius, radius));
    AABB tipAABB(tip, Float3(radius, radius, radius));
    AABB result = AABB::merge(baseAABB, tipAABB);
    FE_CHECK(result.is_valid());
    return result;
}    

}