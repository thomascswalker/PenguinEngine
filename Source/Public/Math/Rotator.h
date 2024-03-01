#pragma once

#include "MathImpl.h"

template <typename T>
struct TRotator
{
    // Rotation around Right axis (Y)
    T Pitch;
    // Rotation around Up axis (Z)
    T Yaw;
    // Rotation around Forward axis (X)
    T Roll;

    TRotator() = default;
    explicit TRotator(const TQuat<T>& Q)
    {
        *this = Q.Rotator();
    }

    explicit TRotator(const TVector3<T>& Euler)
    {
        Pitch = Euler.Y;
        Yaw = Euler.Z;
        Roll = Euler.X;
    }

    T NormalizeAxis(T Angle) const
    {
        Angle = Math::Clamp(Angle, 0, 360);
        if (Angle > static_cast<T>(180.0))
        {
            Angle -= static_cast<T>(360.0);
        }
        return Angle;
    }
    
    void Normalize()
    {
        Pitch = NormalizeAxis(Pitch);
        Yaw = NormalizeAxis(Yaw);
        Roll = NormalizeAxis(Roll);
    }
};
