#pragma once

#include <algorithm>
#include <cmath>
#include <float.h>

#define P_PI 3.14159265358979323846f
#define P_EPSILON FLT_EPSILON
#define P_SINGULARITY_THRESHOLD 0.4999995f
#define P_RAD_TO_DEG (180.0f / P_PI)

enum EAxis
{
    X,
    Y,
    Z
};

namespace Math
{
    template <typename T>
    constexpr static T Sin(T Value)
    {
        return std::sinf(Value);
    }

    template <typename T>
    constexpr static T ASin(T Value)
    {
        return std::asinf(Value);
    }

    template <typename T>
    constexpr static T Cos(T Value)
    {
        return std::cosf(Value);
    }

    template <typename T>
    constexpr static T ACos(T Value)
    {
        return std::acosf(Value);
    }

    template <typename T>
    constexpr static T Tan(T Value)
    {
        return std::tanf(Value);
    }

    template <typename T>
    constexpr static T ATan(T Value)
    {
        return std::atanf(Value);
    }

    template <typename T>
    constexpr static T ATan2(T A, T B)
    {
        return std::atan2f(A, B);
    }

    template <typename T>
    static void SinCos(T& S, T& C, T Value)
    {
        std::sinf(Value);
        std::cosf(C);
    }

    template <typename T>
    constexpr static T Abs(T Value)
    {
        return std::abs(Value);
    }

    template <typename T>
    constexpr static T Min(T A, T B)
    {
        return A < B ? A : B;
    }

    template <typename T>
    constexpr static T Max(T A, T B)
    {
        return A > B ? A : B;
    }

    template <typename T>
    constexpr static T Clamp(T Value, T InMin, T InMax)
    {
        return std::clamp(Value, InMin, InMax);
    }

    template <typename T>
    constexpr static T Square(T Value)
    {
        return Value * Value;
    }

    template <typename T>
    constexpr static T Sqrt(T Value)
    {
        return std::sqrtf(Value);
    }

    template <typename T>
    constexpr static bool IsFinite(T Value)
    {
        return _finite(Value) != 0;
    }

    template <typename T>
    constexpr static bool Sign(T Value)
    {
        return Value >= 0;
    }
};
