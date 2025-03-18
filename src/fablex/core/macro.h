#pragma once

#include <cassert>

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

#define FE_COMPILE_CHECK(exp) static_assert(exp, #exp)
#define FE_CHECK(exp) assert(exp)
#define FE_CHECK_MSG(exp, msg) assert(exp && msg)

#define FE_CONCAT_IMPL(x, y) x##y
#define FE_CONCAT(x, y) FE_CONCAT_IMPL(x, y)

#define FOR_EACH_1(what, x, ...) what(x)
#define FOR_EACH_2(what, x, ...)		\
    what(x);							\
    FOR_EACH_1(what, __VA_ARGS__)
#define FOR_EACH_3(what, x, ...)		\
    what(x);							\
    FOR_EACH_2(what, __VA_ARGS__)
#define FOR_EACH_4(what, x, ...)		\
    what(x);							\
    FOR_EACH_3(what, __VA_ARGS__)
#define FOR_EACH_5(what, x, ...)		\
    what(x);							\
    FOR_EACH_4(what, __VA_ARGS__)
#define FOR_EACH_6(what, x, ...)		\
    what(x);							\
    FOR_EACH_5(what, __VA_ARGS__)
#define FOR_EACH_7(what, x, ...)		\
    what(x);							\
    FOR_EACH_6(what, __VA_ARGS__)
#define FOR_EACH_8(what, x, ...)		\
    what(x);							\
    FOR_EACH_7(what, __VA_ARGS__)
#define FOR_EACH_9(what, x, ...)		\
    what(x);							\
    FOR_EACH_8(what, __VA_ARGS__)
#define FOR_EACH_10(what, x, ...)		\
    what(x);							\
    FOR_EACH_9(what, __VA_ARGS__)
#define FOR_EACH_11(what, x, ...)		\
    what(x);							\
    FOR_EACH_10(what, __VA_ARGS__)
#define FOR_EACH_12(what, x, ...)		\
    what(x);							\
    FOR_EACH_11(what, __VA_ARGS__)
#define FOR_EACH_13(what, x, ...)		\
    what(x);							\
    FOR_EACH_12(what, __VA_ARGS__)
#define FOR_EACH_14(what, x, ...)		\
    what(x);							\
    FOR_EACH_13(what, __VA_ARGS__)
#define FOR_EACH_15(what, x, ...)		\
    what(x);							\
    FOR_EACH_14(what, __VA_ARGS__)
#define FOR_EACH_16(what, x, ...)		\
    what(x);							\
    FOR_EACH_15(what, __VA_ARGS__)
#define FOR_EACH_17(what, x, ...)		\
    what(x);							\
    FOR_EACH_16(what, __VA_ARGS__)
#define FOR_EACH_18(what, x, ...)		\
    what(x);							\
    FOR_EACH_17(what, __VA_ARGS__)
#define FOR_EACH_19(what, x, ...)		\
    what(x);							\
    FOR_EACH_18(what, __VA_ARGS__)
#define FOR_EACH_20(what, x, ...)		\
    what(x);							\
    FOR_EACH_19(what, __VA_ARGS__)

#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__)
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...) N
#define FOR_EACH_RSEQ_N() 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define FOR_EACH_(N, what, ...) CONCATENATE(FOR_EACH_, N)(what, __VA_ARGS__)
#define FE_FOR_EACH(what, ...) FOR_EACH_(FOR_EACH_NARG(__VA_ARGS__), what, __VA_ARGS__)

#define FE_ARRAY_SIZE(Arr) (sizeof(Arr) / sizeof(Arr[0]))
