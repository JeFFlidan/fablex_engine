#ifndef MATH
#define MATH

#define FLOAT_MAX 3.402823466e+38
#define PI 3.14159265359

inline float3x3 adjoint(in float4x4 mat)
{
    // Taken from https://www.shadertoy.com/view/3s33zj
    return float3x3(
        cross(mat[1].xyz, mat[2].xyz), 
        cross(mat[2].xyz, mat[0].xyz), 
        cross(mat[0].xyz, mat[1].xyz)
    );
}

template<typename T>
inline T barycentric_interpolation(in T v0, in T v1, in T v2, in float2 barycentric)
{
    return mad(v0, 1 - barycentric.x - barycentric.y, mad(v1, barycentric.x, v2 * barycentric.y));
}

template<typename T>
inline T barycentric_interpolation(in T v0, in T v1, in T v2, in half2 barycentric)
{
    return mad(v0, 1 - barycentric.x - barycentric.y, mad(v1, barycentric.x, v2 * barycentric.y));
}

inline float3x3 get_tangent_space(in float3 normal)
{
    float3 helper = abs(normal.x) > 0.99 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangent = normalize(cross(normal, helper));
    float3 binormal = normalize(cross(normal, tangent));
    return float3x3(tangent, binormal, normal);
}

inline float3 hemisphere_point_cos(float u, float v)
{
    float phi = v * 2 * PI;
    float cosTheta = sqrt(1 - u);
    float sinTheta = sqrt(1 - cosTheta * cosTheta);
    return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

inline float3 sample_hemisphere_cos(in float3 normal, inout RNG rng)
{
    return mul(hemisphere_point_cos(rng.next_float(), rng.next_float()), get_tangent_space(normal));
}

template<typename T>
float max3(T v)
{
    return max(max(v.x, v.y), v.z);
}

template<typename T>
float min3(T v)
{
    return min(min(v.x, v.y), v.z);
}

#endif // MATH