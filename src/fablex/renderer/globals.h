#pragma once

#include "core/types.h"

namespace fe::renderer
{

inline uint64 g_frameIndex = 0;
inline uint64 g_frameNumber = 0;

constexpr const char* BACK_BUFFER_NAME = "BackBuffer_0451";
constexpr const char* SAMPLER_LINEAR_REPEAT = "SamplerLinearRepeat";
constexpr const char* SAMPLER_LINEAR_CLAMP = "SamplerLinearClamp";
constexpr const char* SAMPLER_LINEAR_MIRROR = "SamplerLinearMirror";
constexpr const char* SAMPLER_NEAREST_REPEAT = "SamplerNearestRepeat";
constexpr const char* SAMPLER_NEAREST_CLAMP = "SamplerNearesClamp";
constexpr const char* SAMPLER_NEAREST_MIRROR = "SamplerNearestMirror";
constexpr const char* SAMPLER_MINIMUM_NEAREST_CLAMP = "SamplerMinimumNearestClamp";

// Must only be used for render graph cross frame resources 
inline uint64 get_cur_frame_index()
{
    return g_frameNumber % 2;
}

// Must only be used for render graph cross frame resources
inline uint64 get_prev_frame_index()
{
    return (g_frameNumber % 2) + 1;
}

}