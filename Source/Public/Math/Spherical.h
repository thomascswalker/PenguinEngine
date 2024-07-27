#pragma once

#include "Vector.h"
#include "Rotator.h"

// https://github.com/mrdoob/three.js/blob/cb24e42a65172ec475ff23a4abe520b724076a24/examples/jsm/controls/OrbitControls.js
struct FSphericalCoords
{
    float Phi = 0.0f;   // Yaw, horizontal angle in radians
    float Theta = 0.0f; // Pitch, vertical angle in radians
    float Radius = 5.0f;

    void MakeSafe(const float Threshold = P_EPSILON)
    {
        Phi = Math::Max(Threshold, Math::Min(P_PI - Threshold, Phi));
    }

    static FSphericalCoords FromCartesian(const float X, const float Y, const float Z)
    {
        FSphericalCoords S;
        S.Radius = Math::Sqrt(X * X + Y * Y + Z * Z);
        if (S.Radius == 0.0f)
        {
            S.Theta = 0.0f;
            S.Phi = 0.0f;
        }
        else
        {
            S.Theta = Math::ATan2(X, Z);
            S.Phi = Math::ACos(Math::Clamp(Y / S.Radius, -1.0f, 1.0f));
        }
        return S;
    }

    FVector3 ToCartesian() const
    {
        const float SinPhiRadius = Math::Sin(Phi) * Radius;
        return FVector3{
            SinPhiRadius * Math::Sin(Theta),
            Math::Cos(Phi) * Radius,
            SinPhiRadius * Math::Cos(Theta)
        };
    }

    static FSphericalCoords FromRotation(const FRotator& Rot)
    {
        FSphericalCoords S;
        S.Theta = Math::DegreesToRadians(Rot.Pitch);
        S.Phi = Math::DegreesToRadians(Rot.Yaw);
        S.MakeSafe();
        return S;
    }

    void RotateRight(const float Angle)
    {
        Theta -= Angle;
    }

    void RotateUp(const float Angle)
    {
        Phi -= Angle;
        MakeSafe();
    }
};
