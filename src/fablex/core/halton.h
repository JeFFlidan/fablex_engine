#pragma once

#include "types.h"
#include "macro.h"
#include <vector>
#include <array>
#include <numeric>

namespace fe
{

class Halton
{
public:
    template<uint32 Dimensionality>
    using Sequence = std::vector<std::array<float, Dimensionality>>;

    static uint32 prime(uint32 n);
    static std::vector<float> sequence(uint32 elementStartIdx, uint32 elementEndIdx);

    template<uint32 Dimensionality>
    static Sequence<Dimensionality> sequence(uint32 elementStartIdx, uint32 elementEndIdx)
    {
        using FloatArray = std::array<float, Dimensionality>;
        using UIntArray = std::array<uint32, Dimensionality>;

        FE_CHECK(elementStartIdx <= elementEndIdx);

        uint32 d = 0;
        uint32 i = elementStartIdx;
        uint32 j = 0;
        uint32 k = 0;
        uint32 n = elementEndIdx - elementStartIdx + 1;

        Sequence<Dimensionality> r;
        FloatArray primeInv;
        UIntArray t; 

        r.resize(n);

        for (FloatArray& innerArray : r)
            innerArray.fill(0.0f);

        for (k = 0; k < n; ++k)
        {
            t.fill(i);

            for (j = 0; j != Dimensionality; ++j)
                primeInv[j] = 1.0f / (float)(prime(j + 1));

            while (std::accumulate(t.begin(), t.end(), 0.0f) > 0)
            {
                for (j = 0; j != Dimensionality; ++j)
                {
                    d = (t[j] % prime(j + 1));
                    r[k][j] = r[k][j] + float(d) * primeInv[j];
                    primeInv[j] = primeInv[j] / float(prime(j + 1));
                    t[j] = (t[j] / prime(j + 1));
                }
            }

            ++i;
        }

        return r;
    }
};

}