#pragma once

#include "core/types.h"

namespace fe::renderer
{

inline uint64 g_frameIndex = 0;

constexpr const char* g_backBufferName = "BackBuffer_0451";
constexpr const char* g_samplerLinearRepeat = "SamplerLinearRepeat";
constexpr const char* g_samplerLinearClamp = "SamplerLinearClamp";
constexpr const char* g_samplerLinearMirror = "SamplerLinearMirror";
constexpr const char* g_samplerNearestRepeat = "SamplerNearestRepeat";
constexpr const char* g_samplerNearestClamp = "SamplerNearesClamp";
constexpr const char* g_samplerNearestMirror = "SamplerNearestMirror";
constexpr const char* g_samplerMinimumNearestClamp = "SamplerMinimumNearestClamp";

}