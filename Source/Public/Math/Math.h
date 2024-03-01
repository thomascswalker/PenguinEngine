#pragma once

#include <cmath>
#include <float.h>

#define P_PI 3.14159265358979323846f
#define P_EPSILON FLT_EPSILON

namespace Math
{
    template <typename T>
    constexpr T Sin(T Value)
    {
        return std::sinf(Value);
    }

    template <typename T>
    constexpr T Cos(T Value)
    {
        return std::cosf(Value);
    }

    template <typename T>
    constexpr T Tan(T Value)
    {
        return std::tanf(Value);
    }

    template <typename T>
    constexpr T Abs(T Value)
    {
        return std::abs(Value);
    }

    template <typename T>
    constexpr T Min(T A, T B)
    {
        return A < B ? A : B;
    }

    template <typename T>
    constexpr T Max(T A, T B)
    {
        return A > B ? A : B;
    }

    template <typename T>
    constexpr bool IsFinite(T Value)
    {
        return _finite(Value) != 0;
    }

    template <typename T>
    constexpr bool Sign(T Value)
    {
        return Value >= 0;
    }
}
