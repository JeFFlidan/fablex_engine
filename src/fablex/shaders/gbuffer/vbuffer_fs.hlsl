#include "gbuffer/vbuffer.hlsli"
#include "object/object_mesh.hlsli"

uint2 main(PixelInput input, nointerpolation MeshletPrimitiveAttributes prim) : SV_TARGET
{
    return pack_vbuffer_data(input, prim.triangleIndex);
}