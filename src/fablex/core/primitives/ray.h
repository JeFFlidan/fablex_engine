#pragma once

#include "core/math.h"

namespace fe
{

class AABB;
class Sphere;
class Capsule;
class Plane;

struct Ray
{
    Float3 origin;
    Float3 direction;
    Float3 directionInverse;
    float tmin = 0;
    float tmax = FLOAT_MAX;

    explicit Ray(
        const Float3& inOrigin = Float3(0, 0, 0),
        const Float3& inDirection = Float3(0, 0, 0),
        float inTMin = 0,
        float inTMax = FLOAT_MAX
    ); 

    explicit Ray(const Vector& inOrigin, const Vector& inDirection, float inTMin = 0, float inTMax = FLOAT_MAX);

    bool intersects(const AABB& aabb) const;
    bool intersects(const Sphere& sphere) const;
    bool intersects(const Sphere& sphere, float& outDistance) const;
    bool intersects(const Sphere& sphere, float& outDistance, Float3& outDirection) const;
    bool intersects(const Capsule& capsule) const;
    bool intersects(const Capsule& capsule, float& outDistance) const;
    bool intersects(const Capsule& capsule, float& outDistance, Float3& outDirection) const;
    bool intersects(const Plane& plane) const;
    bool intersects(const Plane& plane, float& outDistance) const;
    bool intersects(const Plane& plane, float& outDistance, Float3& outDirection) const;

    Float4x4 get_placement_orientation(const Float3& position, const Float3& normal) const;

    static Ray create_from_points(const Float3& a, const Float3& b);
};

}