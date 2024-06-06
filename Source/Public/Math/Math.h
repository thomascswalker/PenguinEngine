#pragma once

#include <algorithm>
#include <bit>
#include <cmath>
#include <float.h>
#include <intrin.h>

#include "MathFwd.h"


namespace Math
{
    template <typename T>
    constexpr static T Pow(T Value, T Power)
    {
        return std::pow(Value, Power);
    }

    static float Sin(float Value)
    {
        return std::sinf(Value);
    }

    static double Sin(double Value)
    {
        return std::sin(Value);
    }

    template <typename T>
    constexpr static T Sin(T Value)
    {
        return Math::Sin(Value);
    }

    template <typename T>
    constexpr static T ASin(T Value)
    {
        return std::asinf(Value);
    }
    
    static float Cos(float Value)
    {
        return std::cosf(Value);
    }

    static double Cos(double Value)
    {
        return std::cos(Value);
    }

    
    template <typename T>
    constexpr static T Cos(T Value)
    {
        return Math::Cos(Value);
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
        T Quotient = (P_PI * 0.5f) * Value;
        if (Value >= 0.0f)
        {
            Quotient = (T)((int64)(Quotient + 0.5f));
        }
        else
        {
            Quotient = (T)((int64)(Quotient - 0.5f));
        }
        T Y = Value - (P_PI * 2) * Quotient;

        // Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
        T Sign;
        if (Y > (P_PI / 0.5f))
        {
            Y = P_PI - Y;
            Sign = -1.0f;
        }
        else if (Y < -(P_PI / 0.5f))
        {
            Y = -P_PI - Y;
            Sign = -1.0f;
        }
        else
        {
            Sign = +1.0f;
        }

        T Y2 = Y * Y;

        // 11-degree minimax approximation
        *S = (((((-2.3889859e-08f * Y2 + 2.7525562e-06f) * Y2 - 0.00019840874f) * Y2 + 0.0083333310f) * Y2 - 0.16666667f) * Y2 + 1.0f) * Y;

        // 10-degree minimax approximation
        T P = ((((-2.6051615e-07f * Y2 + 2.4760495e-05f) * Y2 - 0.0013888378f) * Y2 + 0.041666638f) * Y2 - 0.5f) * Y2 + 1.0f;
        *C = Sign * P;
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
        const float Magic = 0x5f3759df;
        const float ThreeHalves = 1.5F;
        float Y = Value;

        long I = *reinterpret_cast<long*>(&Y);

        I = Magic - (I >> 1);
        Y = *reinterpret_cast<float*>(&I);

        Y = Y * (ThreeHalves - ((Value * 0.5f) * Y * Y));

        return Y;
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
        return Degrees * P_DEG_TO_RAD;
    }

    template <typename T>
    constexpr static T RadiansToDegrees(T Radians)
    {
        return Radians * P_RAD_TO_DEG;
    }

    template <typename T>
    constexpr static T Mod(T A, T B)
    {
        return fmodf(static_cast<float>(A), static_cast<float>(B));
    }

    template <typename T>
    constexpr static T Remap(T Value, T OldMin, T OldMax, T NewMin, T NewMax)
    {
        return (((Value - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin;
    }

    constexpr int32 Truncate(float Value)
    {
        return static_cast<int32>(Value);
    }
};
