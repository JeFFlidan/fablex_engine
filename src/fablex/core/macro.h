#pragma once

// https://meghprkh.github.io/blog/posts/c++-force-inline/#fn5
#if defined(__clang__)
#define FORCE_INLINE [[gnu::always_inline]] [[gnu::gnu_inline]] inline

#elif defined(__GNUC__)
#define FORCE_INLINE [[gnu::always_inline]] inline

#elif defined(_MSC_VER)
#pragma warning(error: 4714)
#define FORCE_INLINE __forceinline

#else
#error Unsupported compiler
#endif // __clang__
