#pragma once

#include "core/math.h"
#include <vector>

namespace fe
{

struct Ray;
struct Sphere;

struct AABB
{
    Float3 minPoint{ FLOAT_MAX, FLOAT_MAX, FLOAT_MAX };
    Float3 maxPoint{ FLOAT_MIN, FLOAT_MIN, FLOAT_MIN };

    enum class IntersectionType
    {
        INSIDE,
        INTERSECTS,
        OUTSIDE,
    };

    AABB() = default;
    AABB(const Sphere& sphere);
    AABB(const std::vector<Float3>& vertexPositions);
    AABB(const Float3& center, const Float3& halfWidth);

    void create(const Sphere& sphere);
    void create(const std::vector<Float3>& vertexPositions);
    void create(const Float3& center, const Float3& halfWidth);

    AABB transform(const Matrix& mat) const;
    AABB transform(const Float4x4& mat) const;

    Matrix get_unorm_remap_matrix() const;
    Float3 get_center() const;
    Float3 get_half_width() const;
    float get_radius() const;

    IntersectionType intersects(const AABB& aabb) const;
    bool intersects(const Float3& point) const;
    bool intersects(const Ray& ray) const;
    bool intersects(const Sphere& sphere) const;

    bool is_valid() const;

    static AABB merge(const AABB& a, const AABB& b);
};

}