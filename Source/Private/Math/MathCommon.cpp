#include "Math/MathCommon.h"

// Matrices

template <typename T>
TVector3<T> TMatrix<T>::GetScale(T Tolerance)
{
    TVector3<T> Scale(0, 0, 0);

    // For each row, find magnitude, and if its non-zero re-scale so its unit length.
    const T SquareSum0 = (M[0][0] * M[0][0]) + (M[0][1] * M[0][1]) + (M[0][2] * M[0][2]);
    const T SquareSum1 = (M[1][0] * M[1][0]) + (M[1][1] * M[1][1]) + (M[1][2] * M[1][2]);
    const T SquareSum2 = (M[2][0] * M[2][0]) + (M[2][1] * M[2][1]) + (M[2][2] * M[2][2]);

    if (SquareSum0 > Tolerance)
    {
        T Scale0 = Math::Sqrt(SquareSum0);
        Scale[0] = Scale0;
        T InvScale0 = 1.f / Scale0;
        M[0][0] *= InvScale0;
        M[0][1] *= InvScale0;
        M[0][2] *= InvScale0;
    }
    else
    {
        Scale[0] = 0;
    }

    if (SquareSum1 > Tolerance)
    {
        T Scale1 = Math::Sqrt(SquareSum1);
        Scale[1] = Scale1;
        T InvScale1 = 1.f / Scale1;
        M[1][0] *= InvScale1;
        M[1][1] *= InvScale1;
        M[1][2] *= InvScale1;
    }
    else
    {
        Scale[1] = 0;
    }

    if (SquareSum2 > Tolerance)
    {
        T Scale2 = Math::Sqrt(SquareSum2);
        Scale[2] = Scale2;
        T InvScale2 = 1.f / Scale2;
        M[2][0] *= InvScale2;
        M[2][1] *= InvScale2;
        M[2][2] *= InvScale2;
    }
    else
    {
        Scale[2] = 0;
    }

    return Scale;
}

// Rotator

template <typename T>
TRotator<T>::TRotator(TQuat<T>& Q)
{
    *this = Q.Rotator();
}

template <typename T>
TRotator<T>::TRotator(const TVector3<T>& Euler)
{
    Pitch = Euler.Y;
    Yaw = Euler.Z;
    Roll = Euler.X;
}

// Rotator to Quaternion
template <typename T>
TQuat<T> TRotator<T>::Quaternion() const
{
    return TQuat<T>(T(Pitch), T(Yaw), T(Roll));
}


// Quat
// Quaternion to Rotator
template <typename T>
TRotator<T> TQuat<T>::Rotator()
{
    T Roll = Math::ATan2(2.0f * Y * W - 2.0f * X * Z, 1.0f - 2.0f * Y * Y - 2.0f * Z * Z);
    T Pitch = Math::ATan2(2.0f * X * W - 2.0f * Y * Z, 1.0f - 2.0f * X * X - 2.0f * Z * Z);
    T Yaw = Math::ASin(2.0f * X * Y + 2.0f * Z * W);

    TRotator Result(Pitch, Yaw, Roll);
    return Result;
}

// Instantiate for the linker
template struct TRotator<float>;
template struct TRotator<double>;
template struct TQuat<float>;
template struct TQuat<double>;
template struct TVector3<float>;
template struct TVector3<double>;
template struct TVector4<float>;
template struct TVector4<double>;
template struct TMatrix<float>;
template struct TMatrix<double>;
