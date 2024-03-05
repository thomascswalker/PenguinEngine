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
        Rotation = TRotator<T>();
        Translation = TVector3<T>();
        Scale = TVector3<T>();
    }
    explicit TTransform(const TVector3<T>& InTranslation)
    {
        Rotation = TRotator<T>();
        Translation = InTranslation;
        Scale = TVector3<T>();
    }
    explicit TTransform(TQuat<T>& InRotation)
    {
        Rotation = InRotation.Rotator();
        Translation = TVector3<T>();
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
    
    TMatrix<T> ToMatrix() const
    {
        TMatrix<T> Out;

        // Apply translation
        Out.M[0][3] = Translation.X;
        Out.M[1][3] = Translation.Y;
        Out.M[2][3] = Translation.Z;

        // Apply rotation
        TMatrix RotationMatrix = TMatrix<T>::MakeRotationMatrix(Rotation);
        for (int Y = 0; Y < 3; ++Y)
        {
            for (int X = 0; X < 3; ++X)
            {
                Out.M[Y][X] = RotationMatrix.M[Y][X];
            }
        }

        // Apply scale
        Out.M[0][3] *= Scale.X;
        Out.M[1][3] *= Scale.Y;
        Out.M[2][3] *= Scale.Z;

        return Out;
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
