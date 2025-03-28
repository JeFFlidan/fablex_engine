#pragma once

#include "core/math.h"

namespace fe
{

struct Sphere;
struct Capsule;
struct Ray;

struct Plane
{
    Float3 origin;
    Float3 normal;
    Float4x4 projection = IDENTITY_MATRIX;

    bool intersects(const Sphere& sphere) const;
    bool intersects(const Sphere& sphere, float& outDistance) const;
    bool intersects(const Sphere& sphere, float& outDistance, Float3& outDirection) const;
    bool intersects(const Capsule& capsule) const;
    bool intersects(const Capsule& capsule, float& outDistance) const;
    bool intersects(const Capsule& capsule, float& outDistance, Float3& outDirection) const;
    bool intersects(const Ray& ray) const;
    bool intersects(const Ray& ray, float& outDistance) const;
    bool intersects(const Ray& ray, float& outDistance, Float3& outDirection) const;
};

}