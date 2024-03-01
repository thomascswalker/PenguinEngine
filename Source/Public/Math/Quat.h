#pragma once

#include "Rotator.h"

template <typename T>
struct alignas(16) TQuat
{
    union
    {
        struct // NOLINT
        {
            T X;
            T Y;
            T Z;
            T W;
        };
        T XYZW[4];
    };

    TQuat(){}
    TQuat(T InX, T InY, T InZ, T InW) : X(InX), Y(InY), Z(InZ), W(InW){}
    TQuat(TVector3<T> Axis, T Angle)
    {
        const T HalfAngle = 0.5f * Angle;
        T S, C;
        Math::SinCos(S, C, HalfAngle);

        X = S * Axis.X;
        Y = S * Axis.Y;
        Z = S * Axis.Z;
        W = C;
    }

    TRotator<T> Rotator()
    {
        // Test for singularities, accounts for gimbal lock
        // https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/
        const T SingularityTest = X * Y + Z * W;
        const T YawX = 1.0f - 2.0f * (Math::Square(Y) + Math::Square(Z));
        const T YawY = 2.0f * (W * Z + X * Y);
        const T RollX = -2.0f * (X * Y * Z * W);
        const T RollY = 1.0f - (2.0f * (X * Y * Z * W));
        const T PitchX = Math::Sqrt(1.0f + 2.0f * (W * Y - X * Z));
        const T PitchY = Math::Sqrt(1.0f - 2.0f * (W * Y - X * Z));
        T Pitch, Yaw, Roll;
    
        if (SingularityTest > P_SINGULARITY_THRESHOLD)
        {
            // North pole
            Pitch = -90.0f;
            Yaw = Math::ATan2(YawX, YawY) * P_RAD_TO_DEG;
        
        }
        if (SingularityTest < -P_SINGULARITY_THRESHOLD)
        {
            // South pole
        }
        else
        {
            // Normal
            Pitch = Math::ASin(2.0f * SingularityTest) * P_RAD_TO_DEG;
            Yaw = Math::ATan2(YawX, YawY) * P_RAD_TO_DEG;
            Roll = Math::ATan2(-YawY, YawX) * P_RAD_TO_DEG;
        }
    
        TRotator<T> Result(Pitch, Yaw, Roll);
        return Result;
    }
};
