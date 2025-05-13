#ifndef RNG_D
#define RNG_D

static const float PNG_01_CONVERT = (1.0f / 4294967296.0f);

struct RNG
{
    uint state;

    uint rand_xorshift()
    {
        state ^= uint(state << 13);
        state ^= uint(state >> 17);
        state ^= uint(state << 5);
        return state;
    }

    uint wang_hash(uint seed)
    {
        seed = (seed ^ 61) ^ (seed >> 16);
        seed *= 9;
        seed = seed ^ (seed >> 4);
        seed *= 0x27d4eb2d;
        seed = seed ^ (seed >> 15);
        return seed;
    }

    void compute_rng_seed(uint index, uint iteration, uint depth)
    {
        state = uint(wang_hash(1 << 31) | (depth << 22) | iteration) ^ wang_hash(index);
    }

    float next_float()
    {
        return float(rand_xorshift() * PNG_01_CONVERT);
    }

    float2 next_float2()
    {
        return float2(next_float(), next_float());
    }

    float3 next_float3()
    {
        return float3(next_float(), next_float(), next_float());
    }
    
    uint next_uint(uint max)
    {
        float f = next_float();
        return uint(floor(f * max));
    }
};

#endif // RNG_D