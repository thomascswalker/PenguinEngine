#pragma once

#include <format>
#include "MathFwd.h"

// Rotator composed of euler angles, in degrees
template <typename T>
struct TRotator
{
    // Rotation around Right axis (Z)
    T Pitch = 0;
    // Rotation around Up axis (Y)
    T Yaw = 0;
    // Rotation around Forward axis (X)
    T Roll = 0;

    TRotator() = default;
    TRotator(TQuat<T>& Q);
    TRotator(T InPitch, T InYaw, T InRoll) : Pitch(InPitch), Yaw(InYaw), Roll(InRoll)
    {
    }
    TRotator(const TVector3<T>& Euler);

    static TRotator Identity() { return TRotator(); }
    T NormalizeAxis(T Angle) const
    {
        T Remainder = Math::Mod(Angle, T(360));
        return Math::Mod(Remainder + T(360), T(360));
    }
    void Normalize()
    {
        Pitch = NormalizeAxis(Pitch);
        Yaw = NormalizeAxis(Yaw);
        Roll = NormalizeAxis(Roll);
    }
    std::string ToString() const { return std::format("[Pitch={}, Yaw={}, Roll={}]", Pitch, Yaw, Roll); }

    TRotator operator+(const TRotator& Other)
    {
        TRotator Out;
        Out.Pitch = Pitch + Other.Pitch;
        Out.Yaw = Yaw + Other.Yaw;
        Out.Roll = Roll + Other.Roll;
        return Out;
    }
    TRotator& operator+=(const TRotator& Other)
    {
        *this = *this + Other;
        return *this;
    }
    TRotator operator*(T Scale) const
    {
        return {Pitch * Scale, Yaw * Scale, Roll * Scale};
    }
};
//
// TQuat<float> TRotator<float>::Quaternion() const;
