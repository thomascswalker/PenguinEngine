#pragma once

#include "Vector.h"

// https://github.com/mrdoob/three.js/blob/cb24e42a65172ec475ff23a4abe520b724076a24/examples/jsm/controls/OrbitControls.js
struct FSphericalCoords
{
    FVector3 Origin;
    float Phi = 0.0f; // Yaw, horizontal angle in radians
    float Theta = 0.0f; // Pitch, vertical angle in radians
    float Radius = 5.0f;

    void MakeSafe()
    {
        Phi = Math::Max(P_EPSILON, Math::Min(P_PI - P_EPSILON, Phi));
    }

    void SetFromCartesian(const float X, const float Y, const float Z)
    {
        Radius = Math::Sqrt(X * X + Y * Y + Z * Z);
        if (Radius == 0.0f)
        {
            Theta = 0.0f;
            Phi = 0.0f;
        }
        else
        {
            Theta = Math::ATan2(X, Z);
            Phi = Math::ACos(Math::Clamp(Y / Radius, -1.0f, 1.0f));
        }
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
