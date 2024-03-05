#include "Math/MathCommon.h"

// Matrices

template <typename T>
TVector3<T> TMatrix<T>::GetScale(T Tolerance)
{
    TVector3 Scale(0, 0, 0);

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

template <typename T>
TQuat<T> TRotator<T>::Quaternion() const
{
    const T DEG_TO_RAD = P_PI / (180.f);
    const T RADS_DIVIDED_BY_2 = DEG_TO_RAD / 2.f;
    T SP, SY, SR;
    T CP, CY, CR;

    const T PitchNoWinding = Math::Mod(Pitch, 360.0f);
    const T YawNoWinding = Math::Mod(Yaw, 360.0f);
    const T RollNoWinding = Math::Mod(Roll, 360.0f);

    Math::SinCos(&SP, &CP, PitchNoWinding * RADS_DIVIDED_BY_2);
    Math::SinCos(&SY, &CY, YawNoWinding * RADS_DIVIDED_BY_2);
    Math::SinCos(&SR, &CR, RollNoWinding * RADS_DIVIDED_BY_2);

    TQuat<T> RotationQuat;
    RotationQuat.X = CR * SP * SY - SR * CP * CY;
    RotationQuat.Y = -CR * SP * CY - SR * CP * SY;
    RotationQuat.Z = CR * CP * SY - SR * SP * CY;
    RotationQuat.W = CR * CP * CY + SR * SP * SY;

    return RotationQuat;
}

template <typename T>
T TRotator<T>::NormalizeAxis(T Angle) const
{
    Angle = Math::Clamp(Angle, 0.0f, 360.0f);
    if (Angle > 180.0f)
    {
        Angle -= 360.0f;
    }
    return Angle;
}

template <typename T>
void TRotator<T>::Normalize()
{
    Pitch = NormalizeAxis(Pitch);
    Yaw = NormalizeAxis(Yaw);
    Roll = NormalizeAxis(Roll);
}

// Quat

template <typename T>
TRotator<T> TQuat<T>::Rotator()
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
        Yaw = 0;
        Roll = 0;
    }
    else if (SingularityTest < -P_SINGULARITY_THRESHOLD)
    {
        // South pole
        Pitch = 90.0f;
        Yaw = 0;
        Roll = 0;
    }
    else
    {
        // Normal
        Pitch = Math::ASin(2.0f * SingularityTest) * P_RAD_TO_DEG;
        Yaw = Math::ATan2(YawX, YawY) * P_RAD_TO_DEG;
        Roll = Math::ATan2(-YawY, YawX) * P_RAD_TO_DEG;
    }

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
