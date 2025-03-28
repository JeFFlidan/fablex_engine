#include "plane.h"
#include "sphere.h"
#include "capsule.h"
#include "ray.h"

namespace fe
{

bool Plane::intersects(const Sphere& sphere) const
{
    float distance;
    Float3 direction;
    return intersects(sphere, distance, direction);
}

bool Plane::intersects(const Sphere& sphere, float& outDistance) const
{
    Float3 direction;
    return intersects(sphere, outDistance, direction);
}

bool Plane::intersects(const Sphere& sphere, float& outDistance, Float3& outDirection) const
{
    Vector C(sphere.center);
    outDistance = get_plane_point_distance(C, origin, normal);
    outDirection = normal;

    if (outDistance < 0)
    {
        outDirection.x *= -1;
        outDirection.y *= -1;
        outDirection.z *= -1;
        outDistance = abs(outDistance);
    }

    outDistance -= sphere.radius;

    if (outDistance < 0)
    {
        Matrix planeProjection(projection);
        Vector clipSpacePos = Vector3::transform(C, planeProjection);
        
        Vector uvw = clipSpacePos 
            * Vector4::create(0.5f, -0.5f, 0.5f, 1) 
            + Vector4::create(0.5f, 0.5f, 0.5f, 0);

        Vector uvwSaturated = uvw.saturate();
        Vector uvwDiff = (uvw - uvwSaturated).abs();

        if (uvwDiff.x() > EPSILON)
            outDistance = 0;
        if (uvwDiff.y() > EPSILON)
            outDistance = 0;
        if (uvwDiff.z() > EPSILON)
            outDistance = 0;
    }

    return outDistance < 0;
}

bool Plane::intersects(const Capsule& capsule) const
{
    float distance;
    Float3 direction;
    return intersects(capsule, distance, direction);
}

bool Plane::intersects(const Capsule& capsule, float& outDistance) const
{
    Float3 direction;
    return intersects(capsule, outDistance, direction);
}

bool Plane::intersects(const Capsule& capsule, float& outDistance, Float3& outDirection) const
{
    outDistance = 0;
    outDirection = normal;

    Vector N(normal);
    Vector O(origin);

    Vector A(capsule.base);
    Vector B(capsule.tip);
    Vector D = Vector3::normalize(A - B);

    A -= D * capsule.radius;
    B += D * capsule.radius;

    Vector C;
    if (std::abs(Vector3::dot(N, D)) < EPSILON)
    {
        C = A;
    }
    else
    {
        float t = Vector3::dot(N, A - O) / std::abs(Vector3::dot(N, D));
        Vector linePlaneIntersection = A + D * t;
        C = closest_point_on_line_segment(linePlaneIntersection, A, B);
    }

    outDistance = get_plane_point_distance(C, O, N);

    if (outDistance < 0)
    {
        outDirection.x *= -1;
        outDirection.y *= -1;
        outDirection.z *= -1;
        outDistance = abs(outDistance);
    }

    outDistance -= capsule.radius;

    if (outDistance < 0)
    {
        Matrix planeProjection(projection);
        Vector clipSpacePos = Vector3::transform(C, planeProjection);
        
        Vector uvw = clipSpacePos 
            * Vector4::create(0.5f, -0.5f, 0.5f, 1) 
            + Vector4::create(0.5f, 0.5f, 0.5f, 0);

        Vector uvwSaturated = uvw.saturate();
        Vector uvwDiff = (uvw - uvwSaturated).abs();

        if (uvwDiff.x() > EPSILON)
            outDistance = 0;
        if (uvwDiff.y() > EPSILON)
            outDistance = 0;
        if (uvwDiff.z() > EPSILON)
            outDistance = 0;
    }

    return outDistance < 0;
}

bool Plane::intersects(const Ray& ray) const
{
    float distance;
    Float3 direction;
    return intersects(ray, distance, direction);
}

bool Plane::intersects(const Ray& ray, float& outDistance) const
{
    Float3 direction;
    return intersects(ray, outDistance, direction);
}

bool Plane::intersects(const Ray& ray, float& outDistance, Float3& outDirection) const
{
    outDistance = 0;
    outDirection = normal;

    Vector N(normal);
    Vector D(ray.direction);

    float dotND = Vector3::dot(N, D);

    if (std::abs(dotND) < EPSILON)
        return false;

    Vector O(ray.origin);
    Vector A = O + D * ray.tmin;
    Vector B = O + D * ray.tmax;

    outDistance = Vector3::dot(N, (origin.to_vector() - O) / dotND);
    if (outDistance <= 0)
        return false;

    Vector C = O + D * outDistance;

    Matrix planeProjection(projection);
    Vector clipSpacePos = Vector3::transform(C, planeProjection);
    
    Vector uvw = clipSpacePos 
        * Vector4::create(0.5f, -0.5f, 0.5f, 1) 
        + Vector4::create(0.5f, 0.5f, 0.5f, 0);

    Vector uvwSaturated = uvw.saturate();
    Vector uvwDiff = (uvw - uvwSaturated).abs();

    if (uvwDiff.x() > EPSILON)
        return false;
    if (uvwDiff.y() > EPSILON)
        return false;
    if (uvwDiff.z() > EPSILON)
        return false;

    return true;
}

}