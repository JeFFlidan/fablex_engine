#pragma once

#include "core/math.h"
#include <vector>

namespace fe
{

struct AABB;
struct Capsule;
struct Ray;
struct Plane;
	
struct Sphere
{
    Float3 center{ 0, 0, 0 };
    float radius{ 0 };

    Sphere() = default;
    Sphere(const std::vector<Float3>& vertexPositions);
    Sphere(const AABB& aabb);

    void create(const std::vector<Float3>& vertexPositions);
    void create(const AABB& aabb);

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
    bool intersects(const Ray& ray) const;
    bool intersects(const Ray& ray, float& outDistance) const;
    bool intersects(const Ray& ray, float& outDistance, Float3& outDirection) const;
};

}