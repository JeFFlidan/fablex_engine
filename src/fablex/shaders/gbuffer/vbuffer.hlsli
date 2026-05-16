#ifndef GBUFFER_COMMON_H
#define GBUFFER_COMMON_H

#define OBJECT_VBUFFER
#include "object/object.hlsli"

struct VBufferData
{
    uint instanceID;
    uint meshletID;
    uint triangleID;
};

uint2 pack_vbuffer_data(PixelInput input, uint localTriangleID)
{
    uint2 data;
    data.x = input.instanceID;
    data.y = (input.meshletID << 8) | (localTriangleID & 0xFF);
    return data;
}

VBufferData unpack_vbuffer_data(uint2 packedData)
{
    VBufferData data;
    data.instanceID = packedData.x;
    data.meshletID = packedData.y >> 8;
    data.triangleID = packedData.y & 0xFF;
    return data;
}

#endif // GBUFFER_COMMON_H