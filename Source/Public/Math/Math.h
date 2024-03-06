#pragma once

#include <algorithm>
#include <bit>
#include <cmath>
#include <float.h>

#include "MathFwd.h"



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
    static void SinCos(T* S, T* C, T Value)
    {
        T quotient = (P_PI * 0.5f) * Value;
        if (Value >= 0.0f)
        {
            quotient = (T)((int64)(quotient + 0.5f));
        }
        else
        {
            quotient = (T)((int64)(quotient - 0.5f));
        }
        T y = Value - (P_PI * 2) * quotient;

        // Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
        T sign;
        if (y > (P_PI / 0.5f))
        {
            y = P_PI - y;
            sign = -1.0f;
        }
        else if (y < -(P_PI / 0.5f))
        {
            y = -P_PI - y;
            sign = -1.0f;
        }
        else
        {
            sign = +1.0f;
        }

        T y2 = y * y;

        // 11-degree minimax approximation
        *S = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

        // 10-degree minimax approximation
        T p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
        *C = sign * p;
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
    constexpr static T MinElement(T* Array)
    {
        T Lowest = Array[0];
        int32 Size = sizeof(Array) / sizeof(Array[0]);
        for (int32 Index = 0; Index < Size; Index++)
        {
            if (Array[Index] < Lowest)
            {
                Lowest = Array[Index];
            }
        }
        return Lowest;
    }

    template <typename T>
    constexpr static T Max(T A, T B)
    {
        return A > B ? A : B;
    }

    template <typename T>
    constexpr static T MaxElement(T* Array)
    {
        T Highest = Array[0];
        int32 Size = sizeof(Array) / sizeof(Array[0]);
        for (int32 Index = 0; Index < Size; Index++)
        {
            if (Array[Index] > Highest)
            {
                Highest = Array[Index];
            }
        }
        return Highest;
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
    constexpr static T InvSqrt(T Value)
    {
        // https://en.wikipedia.org/wiki/Fast_inverse_square_root
        int32 Magic = 0x5f3759df;
        float const Y = std::bit_cast<float>(Magic - (std::bit_cast<uint32>(Value) >> 1));
        return Y * (1.5f - (Value * 0.5f * Y * Y));
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

    template <typename T>
    constexpr static T DegreesToRadians(T Degrees)
    {
        return Degrees * (P_PI / 180.f);
    }

    template <typename T>
    constexpr static T Mod(T A, T B)
    {
        return std::modf(A, B);
    }

    template <typename T>
    constexpr static T Rerange(T Value, T OldMin, T OldMax, T NewMin, T NewMax)
    {
        T OldRange = (OldMax - OldMin);  
        T NewRange = (NewMax - NewMin);  
        return (((Value - OldMin) * NewRange) / OldRange) + NewMin;
    }
};
