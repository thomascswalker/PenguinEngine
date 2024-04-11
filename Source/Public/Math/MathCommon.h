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
}
