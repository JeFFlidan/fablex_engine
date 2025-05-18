#ifndef SVGF_COMMON
#define SVGF_COMMON

float luminance(float3 rgb)
{
    return dot(rgb, float3(0.2126, 0.7152, 0.0722));
}

float compute_weight(
    float depthCenter,
    float depthP,
    float phiDepth,
    float3 normalCenter,
    float3 normalP,
    float phiNormal,
    float lumIlluminationCenter,
    float lumIlluminationP,
    float phiIllumination
)
{
    const float weightNormal = pow(saturate(dot(normalCenter, normalP)), phiNormal);
    const float weightDepth = (phiDepth == 0) ? 0.0 : abs(depthCenter - depthP) / phiDepth;
    const float weightLumIllum = abs(lumIlluminationCenter - lumIlluminationP) / phiIllumination;

    const float weightIllum = exp(0.0 - max(weightLumIllum, 0.0) - max(weightDepth, 0.0)) * weightNormal;

    return weightIllum;
}

#endif // SVGF_COMMON