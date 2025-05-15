#ifndef PIXEL_PACKING
#define PIXEL_PACKING

/* 
 * 32-bit HDR color format
 */

// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Shaders/PixelPacking_R11G11B10.hlsli
uint pack_R11G11B10(float3 rgb)
{
    rgb = min(rgb, asfloat(0x477C0000));
    uint r = ((f32tof16(rgb.x) + 8) >> 4) & 0x000007FF;
    uint g = ((f32tof16(rgb.y) + 8) << 7) & 0x003FF800;
    uint b = ((f32tof16(rgb.z) + 16) << 17) & 0xFFC00000;
    return r | g | b;
}

// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Shaders/PixelPacking_R11G11B10.hlsli
float3 unpack_R11G11B10(uint rgb)
{
    float r = f16tof32((rgb << 4 ) & 0x7FF0);
    float g = f16tof32((rgb >> 7 ) & 0x7FF0);
    float b = f16tof32((rgb >> 17) & 0x7FE0);
    return float3(r, g, b);
}

#endif // PIXEL_PACKING