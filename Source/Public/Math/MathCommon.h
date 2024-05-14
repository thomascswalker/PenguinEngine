#pragma once

#include "MathFwd.h"
#include "Color.h"
#include "Matrix.h"
#include "Plane.h"
#include "Quat.h"
#include "Rect.h"
#include "Rotator.h"
#include "Transform.h"
#include "Vector.h"

namespace Math
{
    // Rotate a vector with the specified quaternion.
    // https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl#L18
    template <typename T>
    TVector3<T> Rotate(const TVector3<T>& V, const TQuat<T> Q)
    {
        TMatrix<T> M;
        const T A = Q.W; // Angle is the W component of the quaternion
        const T C = Math::Cos(A); // Cosine of Angle
        const T S = Math::Sin(A); // Sin of Angle

        TVector3<T> Normal = {Q.X, Q.Y, Q.Z};
        TVector3<T> Axis = Normal.Normalized();

        TMatrix<T> Rotation = TMatrix<T>::GetIdentity();
        T One = static_cast<T>(1);
        T Zero = static_cast<T>(0);
        Rotation.Set(0, 0, C + (One - C) * Axis.X * Axis.X);
        Rotation.Set(0, 1, (One - C) * Axis.X * Axis.Y + S * Axis.Z);
        Rotation.Set(0, 2, (One - C) * Axis.X * Axis.Z - S * Axis.Y);
        Rotation.Set(0, 3, Zero);
        Rotation.Set(1, 0, (One - C) * Axis.Y * Axis.X - S * Axis.Z);
        Rotation.Set(1, 1, C + (One - C) * Axis.Y * Axis.Y);
        Rotation.Set(1, 2, (One - C) * Axis.Y * Axis.Z + S * Axis.X);
        Rotation.Set(1, 3, Zero);
        Rotation.Set(2, 0, (One - C) * Axis.Z * Axis.X + S * Axis.Y);
        Rotation.Set(2, 1, (One - C) * Axis.Z * Axis.Y - S * Axis.X);
        Rotation.Set(2, 2, C + (One - C) * Axis.Z * Axis.Z);
        Rotation.Set(2, 3, Zero);

        TTranslationMatrix<T> Translation(V);
        TMatrix<T> Out = Translation * Rotation;
        return Out.GetTranslation();
    }

    // Rotate a vector with the specified Pitch, Yaw, and Roll (in degrees)
    template <typename T>
    TVector3<T> Rotate(const TVector3<T>& V, T Pitch, T Yaw, T Roll)
    {
        // Convert degrees to radians
        T PitchRad = Math::DegreesToRadians(Pitch);
        T YawRad = Math::DegreesToRadians(Yaw);
        T RollRad = Math::DegreesToRadians(Roll);

        // Construct a quaternion from euler angles
        TQuat<T> Q(PitchRad, YawRad, RollRad);

        // Rotate the vector with the quaternion and return the result
        return Rotate(V, Q);
    }

    // Rotator to Quaternion
    template <typename T>
    TQuat<T> ToQuat(TRotator<T> R)
    {
        return TQuat<T>(T(R.Pitch), T(R.Yaw), T(R.Roll));
    }

    // Quaternion to Rotator
    template <typename T>
    TRotator<T> ToRot(TQuat<T> Q)
    {
        T Roll = Math::ATan2(2.0f * Q.Y * Q.W - 2.0f * Q.X * Q.Z, 1.0f - 2.0f * Q.Y * Q.Y - 2.0f * Q.Z * Q.Z);
        T Pitch = Math::ATan2(2.0f * Q.X * Q.W - 2.0f * Q.Y * Q.Z, 1.0f - 2.0f * Q.X * Q.X - 2.0f * Q.Z * Q.Z);
        T Yaw = Math::ASin(2.0f * Q.X * Q.Y + 2.0f * Q.Z * Q.W);

        TRotator Result(Pitch, Yaw, Roll);
        return Result;
    }

    /*
     * Transform the specified TVector4 by the specified TMatrix.
     */
    template <typename T>
    TVector4<T> VectorTransformMatrix(const TVector4<T>& V, const TMatrix<T>& M)
    {
        return FVector4{
            (V.X * M.M[0][0]) + (V.Y * M.M[1][0]) + (V.Z * M.M[2][0]) + (V.W * M.M[3][0]),
            (V.X * M.M[0][1]) + (V.Y * M.M[1][1]) + (V.Z * M.M[2][1]) + (V.W * M.M[3][1]),
            (V.X * M.M[0][2]) + (V.Y * M.M[1][2]) + (V.Z * M.M[2][2]) + (V.W * M.M[3][2]),
            (V.X * M.M[0][3]) + (V.Y * M.M[1][3]) + (V.Z * M.M[2][3]) + (V.W * M.M[3][3])
        };
    }

    template <typename T>
    T Orient2D(const TVector3<T>& A, const TVector3<T>& B, const TVector3<T>& C)
    {
        return (B.X - A.X) * (C.Y - A.Y) - (B.Y - A.Y) * (C.X - A.X);
    }
}
