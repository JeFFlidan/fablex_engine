#pragma once

#include "types.h"
#include "flags_operations.h"

namespace fe
{

enum class Key : uint64
{
    UNKNOWN = 0,
    A = 1ULL << 0,
    B = 1ULL << 1,
    C = 1ULL << 2,
    D = 1ULL << 3,
    E = 1ULL << 4,
    F = 1ULL << 5,
    G = 1ULL << 6,
    H = 1ULL << 7,
    I = 1ULL << 8,
    J = 1ULL << 9,
    K = 1ULL << 10,
    L = 1ULL << 11,
    M = 1ULL << 12,
    N = 1ULL << 13,
    O = 1ULL << 14,
    P = 1ULL << 15,
    Q = 1ULL << 16,
    R = 1ULL << 17,
    S = 1ULL << 18,
    T = 1ULL << 19,
    U = 1ULL << 20,
    V = 1ULL << 21,
    W = 1u << 22,
    X = 1u << 23,
    Y = 1u << 24,
    Z = 1u << 25,
    LEFT_CTRL = 1ULL << 26,
    LEFT_ALT = 1ULL << 27,
    LEFT_SHIFT = 1ULL << 28,
    TAB = 1ULL << 29,
    SPACE = 1ULL << 30,
    ESCAPE = 1ULL << 31,
    LEFT = 1ULL << 32,
    RIGHT = 1ULL << 33,
    UP = 1ULL << 34,
    DOWN = 1ULL << 35,
    _1 = 1ULL << 36,
    _2 = 1ULL << 37,
    _3 = 1ULL << 38,
    _4 = 1ULL << 39,
    _5 = 1ULL << 40,
    _6 = 1ULL << 41,
    _7 = 1ULL << 42,
    _8 = 1ULL << 43,
    _9 = 1ULL << 44,
    _0 = 1ULL << 45,
    ENTER = 1ULL << 46,
    F1 = 1ULL << 47,
    F2 = 1ULL << 48,
    F3 = 1ULL << 49,
    F4 = 1ULL << 50,
    F5 = 1ULL << 51,
    F6 = 1ULL << 52,
    F7 = 1ULL << 53,
    F8 = 1ULL << 54,
    F9 = 1ULL << 55,
    F10 = 1ULL << 56,
    F11 = 1ULL << 57,
    F12 = 1ULL << 58
};

enum class MouseButton
{
    UNKNOWN = 0,
    LEFT = 1 << 0,
    MIDDLE = 1 << 1,
    RIGHT = 1 << 2
};

}

template<>
struct EnableBitMaskOperator<fe::Key>
{
	static const bool enable = true;
};

template<>
struct EnableBitMaskOperator<fe::MouseButton>
{
	static const bool enable = true;
};

