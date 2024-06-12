#pragma once

#include "Framework/Core/Logging.h"
#include "Rotator.h"
#include "Vector.h"

template <typename T>
struct TTransform
{
    TVector3<T> Translation;
    TRotator<T> Rotation;
    TVector3<T> Scale;

    TTransform()
    {
        Rotation = TRotator<T>(0, 0, 0);
        Translation = TVector3<T>(0);
        Scale = TVector3<T>(1);
    }
    explicit TTransform(const TVector3<T>& InTranslation)
    {
        Rotation = TRotator<T>();
        Translation = InTranslation;
        Scale = TVector3<T>();
    }
    explicit TTransform(TRotator<T>& InRotation)
    {
        Rotation = InRotation;
        Translation = TVector3<T>();
        Scale = TVector3<T>();
    }
    TTransform(TQuat<T>& InRotation, const TVector3<T>& InTranslation, const TVector3<T>& InScale = {1.0f, 1.0f, 1.0f})
    {
        Rotation = InRotation.Rotator();
        Translation = InTranslation;
        Scale = InScale;
    }

    TTransform(TRotator<T>& InRotation, const TVector3<T>& InTranslation, const TVector3<T>& InScale = {1.0f, 1.0f, 1.0f})
    {
        Rotation = InRotation;
        Translation = InTranslation;
        Scale = InScale;
    }

    void FromMatrix(TMatrix<T>& InMatrix)
    {
        // Extract scale
        Scale = InMatrix.GetScale();

        // Handle negative scaling
        LOG_WARNING("Implement negative scale handling in TTransform::FromMatrix")

        // Extract rotation
        TQuat<T> InRotation = TQuat(InMatrix);
        Rotation = InRotation.Rotator();

        // Extract translation
        Translation = InMatrix.GetTranslation();

        Rotation.Normalize();
    }

    //  | rx0   | rx1   | rx2   | 0 |
    //  | ry0   | ry1   | ry2   | 0 |
    //  | rz0   | rz1   | rz2   | 0 |
    //  | tx*sx | ty*sy | tz*sz | 1 | 
    TMatrix<T> ToMatrix() const
    {
        // Apply translation
        TMatrix<T> Out = TTranslationMatrix<T>(Translation);

        // Apply rotation
        TMatrix RotationMatrix = TRotationMatrix<T>(Rotation);
        for (int32 X = 0; X < 3; ++X)
        {
            for (int32 Y = 0; Y < 3; ++Y)
            {
                Out.M[Y][X] = RotationMatrix.M[Y][X];
            }
        }

        // Apply scale
        Out.M[3][0] *= Scale.X;
        Out.M[3][1] *= Scale.Y;
        Out.M[3][2] *= Scale.Z;

        return Out;
    }

    TVector3<T> GetAxisNormalized(EAxis InAxis) const
    {
        TMatrix<T> M = ToMatrix().GetInverse();
        TVector3<T> AxisVector = M.GetAxis(InAxis);
        AxisVector.Normalize();
        return AxisVector;
    }

    std::string ToString() const { return std::format("Translation={}, Rotation={}, Scale={}", Translation.ToString(), Rotation.ToString(), Scale.ToString()); }

    TTransform operator*(const TTransform& Other)
    {
        TTransform Out;

        Out.Translation = Translation * Other.Translation;
        Out.Rotation = Other.Rotation;
        Out.Scale = Scale * Other.Scale;

        return Out;
    }
    TTransform& operator*=(const TTransform& Other)
    {
        *this = *this * Other;
        return *this;
    }
};
