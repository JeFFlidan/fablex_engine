#pragma once

#include "core/math.h"

namespace fe
{

struct Plane;
struct Ray;
struct Sphere;
struct AABB;

struct Capsule
{
    Float3 base = Float3(0, 0, 0);
    Float3 tip = Float3(0, 0, 0);
    float radius = 0;

    Capsule() = default;
    Capsule(const AABB& aabb, float height);
    Capsule(const Sphere& sphere, float height);
    Capsule(const Float3& inBase, const Float3& inTip, float inRadius);

    void create(const AABB& aabb, float height);
    void create(const Sphere& sphere, float height);
    void create(const Float3& inBase, const Float3& inTip, float inRadius);

    bool intersects(const Capsule& other, Float3& outPosition, Float3& outIncidentNormal, float& outPenetrationDepth) const;
    bool intersects(const Sphere& sphere) const;
    bool intersects(const Sphere& sphere, float& outDistance) const;
    bool intersects(const Sphere& sphere, float& outDistance, Float3& outDirection) const;
    bool intersects(const Plane& plane) const;
    bool intersects(const Plane& plane, float& outDistance) const;
    bool intersects(const Plane& plane, float& outDistance, Float3& outDirection) const;
    bool intersects(const Ray& ray) const;
    bool intersects(const Ray& ray, float& outDistance) const;
    bool intersects(const Ray& ray, float& outDistance, Float3& outDirection) const;

    AABB get_aabb() const;
};

}