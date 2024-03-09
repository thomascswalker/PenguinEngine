﻿#pragma once

#include <string>

#include "Math.h"
#include "Plane.h"
#include "Rotator.h"
#include "Vector.h"
#include "Framework/Core/Logging.h"

// M[RowIndex][ColumnIndex]
template <typename T>
struct TMatrix
{
    static_assert(std::is_floating_point_v<T>, "Type is not floating point.");

    alignas(16) T M[4][4];

    TMatrix()
    {
        SetIdentity();
        CheckNaN();
    }

    TMatrix(const TPlane<T>& InX, const TPlane<T>& InY, const TPlane<T>& InZ, const TPlane<T>& InW)
    {
        M[0][0] = InX.X;
        M[0][1] = InX.Y;
        M[0][2] = InX.Z;
        M[0][3] = InX.W;
        M[1][0] = InY.X;
        M[1][1] = InY.Y;
        M[1][2] = InY.Z;
        M[1][3] = InY.W;
        M[2][0] = InZ.X;
        M[2][1] = InZ.Y;
        M[2][2] = InZ.Z;
        M[2][3] = InZ.W;
        M[3][0] = InW.X;
        M[3][1] = InW.Y;
        M[3][2] = InW.Z;
        M[3][3] = InW.W;
        CheckNaN();
    }

    template <typename T>
    TMatrix(const TVector3<T>& InX, const TVector3<T>& InY, const TVector3<T>& InZ, const TVector3<T>& InW)
    {
        M[0][0] = InX.X;
        M[0][1] = InX.Y;
        M[0][2] = InX.Z;
        M[0][3] = 0.0f;
        M[1][0] = InY.X;
        M[1][1] = InY.Y;
        M[1][2] = InY.Z;
        M[1][3] = 0.0f;
        M[2][0] = InZ.X;
        M[2][1] = InZ.Y;
        M[2][2] = InZ.Z;
        M[2][3] = 0.0f;
        M[3][0] = InW.X;
        M[3][1] = InW.Y;
        M[3][2] = InW.Z;
        M[3][3] = 1.0f;
        CheckNaN();
    }
    TMatrix(const TMatrix& Other)
    {
        std::memcpy(M, &Other.M, 16 * sizeof(T));
        CheckNaN();
    }
    TMatrix(TMatrix&& Other) noexcept
    {
        std::memcpy(M, &Other.M, 16 * sizeof(T));
        CheckNaN();
    }

    void CheckNaN() const
    {
        for (int32 X = 0; X < 4; X++)
        {
            for (int32 Y = 0; Y < 4; Y++)
            {
                if (!Math::IsFinite(M[X][Y]))
                {
                    LOG_ERROR("Matrix cell [{},{}] contains NaN", X, Y)
                }
            }
        }
    }

    // https://www.opengl-tutorial.org/assets/faq_quaternions/index.html#Q28
    static TMatrix MakeRotationMatrix(const TRotator<T>& Rotation)
    {
        T A = Math::Cos(Rotation.Pitch);
        T B = Math::Sin(Rotation.Pitch);
        T C = Math::Cos(Rotation.Yaw);
        T D = Math::Sin(Rotation.Yaw);
        T E = Math::Cos(Rotation.Roll);
        T F = Math::Sin(Rotation.Roll);

        T AD = A * D;
        T BD = B * D;

        TMatrix Out;
        Out.SetIdentity();

        //      |  CE      -CF       D   0 |
        // M  = |  BDE+AF  -BDF+AE  -BC  0 |
        //      | -ADE+BF   ADF+BE   AC  0 |
        //      |  0        0        0   1 |

        Out.M[0][0] = C * E;
        Out.M[0][1] = -C * F;
        Out.M[0][2] = D;

        Out.M[1][0] = BD * E + A * F;
        Out.M[1][1] = -BD * F + A * E;
        Out.M[1][2] = -B * C;

        Out.M[2][0] = -AD * E + B * F;
        Out.M[2][1] = AD * F + B * E;
        Out.M[2][2] = A * C;

        return Out;
    }

    static TMatrix MakeRotationTranslationMatrix(const FRotator& Rot, const FVector3& Translation)
    {
        T SP, SY, SR;
        T CP, CY, CR;
        Math::SinCos(&SP, &CP, (T)Math::DegreesToRadians(Rot.Pitch));
        Math::SinCos(&SY, &CY, (T)Math::DegreesToRadians(Rot.Yaw));
        Math::SinCos(&SR, &CR, (T)Math::DegreesToRadians(Rot.Roll));

        TMatrix Out;
        
        Out.M[0][0] = CP * CY;
        Out.M[0][1] = CP * SY;
        Out.M[0][2] = SP;
        Out.M[0][3] = 0.f;

        Out.M[1][0] = SR * SP * CY - CR * SY;
        Out.M[1][1] = SR * SP * SY + CR * CY;
        Out.M[1][2] = -SR * CP;
        Out.M[1][3] = 0.f;

        Out.M[2][0] = -(CR * SP * CY + SR * SY);
        Out.M[2][1] = CY * SR - CR * SP * SY;
        Out.M[2][2] = CR * CP;
        Out.M[2][3] = 0.f;

        Out.M[3][0] = Translation.X;
        Out.M[3][1] = Translation.Y;
        Out.M[3][2] = Translation.Z;
        Out.M[3][3] = 1.f;

        return Out;
    }

    // Functions

    bool Equals(const TMatrix& Other, T Threshold = 0.00001f) const
    {
        for (int32 X = 0; X < 4; X++)
        {
            for (int32 Y = 0; Y < 4; Y++)
            {
                if (M[X][Y] - Other.M[X][Y] > Threshold)
                {
                    return false;
                }
            }
        }
        return true;
    }

    static TMatrix GetIdentity()
    {
        return TMatrix{
            TPlane<T>(1, 0, 0, 0),
            TPlane<T>(0, 1, 0, 0),
            TPlane<T>(0, 0, 1, 0),
            TPlane<T>(0, 0, 0, 1)
        };
    }

    void SetIdentity()
    {
        M[0][0] = 1;
        M[0][1] = 0;
        M[0][2] = 0;
        M[0][3] = 0;
        M[1][0] = 0;
        M[1][1] = 1;
        M[1][2] = 0;
        M[1][3] = 0;
        M[2][0] = 0;
        M[2][1] = 0;
        M[2][2] = 1;
        M[2][3] = 0;
        M[3][0] = 0;
        M[3][1] = 0;
        M[3][2] = 0;
        M[3][3] = 1;
    }

    constexpr T GetDeterminant() const
    {
        T A11 = M[0][0];
        T A12 = M[0][1];
        T A13 = M[0][2];
        T A14 = M[0][3];
        T A21 = M[1][0];
        T A22 = M[1][1];
        T A23 = M[1][2];
        T A24 = M[1][3];
        T A31 = M[2][0];
        T A32 = M[2][1];
        T A33 = M[2][2];
        T A34 = M[2][3];
        T A41 = M[3][0];
        T A42 = M[3][1];
        T A43 = M[3][2];
        T A44 = M[3][3];

        T Det1 = A11 * A22 * A33 * A44;
        T Det2 = A11 * A23 * A34 * A42;
        T Det3 = A11 * A24 * A32 * A43;

        T Det4 = A12 * A21 * A34 * A43;
        T Det5 = A12 * A23 * A31 * A44;
        T Det6 = A12 * A24 * A33 * A41;

        T Det7 = A13 * A21 * A32 * A44;
        T Det8 = A13 * A22 * A34 * A41;
        T Det9 = A13 * A24 * A31 * A42;

        T Det10 = A14 * A21 * A33 * A42;
        T Det11 = A14 * A22 * A31 * A43;
        T Det12 = A14 * A23 * A32 * A41;

        T Det13 = A11 * A22 * A34 * A43;
        T Det14 = A11 * A23 * A32 * A44;
        T Det15 = A11 * A24 * A33 * A42;

        T Det16 = A12 * A21 * A33 * A44;
        T Det17 = A12 * A23 * A34 * A41;
        T Det18 = A12 * A24 * A31 * A43;

        T Det19 = A13 * A21 * A34 * A42;
        T Det20 = A13 * A22 * A31 * A44;
        T Det21 = A13 * A24 * A32 * A41;

        T Det22 = A14 * A21 * A32 * A43;
        T Det23 = A14 * A22 * A33 * A41;
        T Det24 = A14 * A23 * A31 * A42;

        return (
            Det1 + Det2 + Det3 +
            Det4 + Det5 + Det6 +
            Det7 + Det8 + Det9 +
            Det10 + Det11 + Det12 -
            Det13 - Det14 - Det15 -
            Det16 - Det17 - Det18 -
            Det19 - Det20 - Det21 -
            Det22 - Det23 - Det24
        );
    }

    // http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche0023.html
    TMatrix GetInverse()
    {
        T Determinant = GetDeterminant();
        if (Determinant == 0.0 || !Math::IsFinite(Determinant))
        {
            return GetIdentity();
        }
        Determinant = 1.0f / Determinant;

        T A2323 = M[2][2] * M[3][3] - M[2][3] * M[3][2];
        T A1323 = M[2][1] * M[3][3] - M[2][3] * M[3][1];
        T A1223 = M[2][1] * M[3][2] - M[2][2] * M[3][1];
        T A0323 = M[2][0] * M[3][3] - M[2][3] * M[3][0];
        T A0223 = M[2][0] * M[3][2] - M[2][2] * M[3][0];
        T A0123 = M[2][0] * M[3][1] - M[2][1] * M[3][0];
        T A2313 = M[1][2] * M[3][3] - M[1][3] * M[3][2];
        T A1313 = M[1][1] * M[3][3] - M[1][3] * M[3][1];
        T A1213 = M[1][1] * M[3][2] - M[1][2] * M[3][1];
        T A2312 = M[1][2] * M[2][3] - M[1][3] * M[2][2];
        T A1312 = M[1][1] * M[2][3] - M[1][3] * M[2][1];
        T A1212 = M[1][1] * M[2][2] - M[1][2] * M[2][1];
        T A0313 = M[1][0] * M[3][3] - M[1][3] * M[3][0];
        T A0213 = M[1][0] * M[3][2] - M[1][2] * M[3][0];
        T A0312 = M[1][0] * M[2][3] - M[1][3] * M[2][0];
        T A0212 = M[1][0] * M[2][2] - M[1][2] * M[2][0];
        T A0113 = M[1][0] * M[3][1] - M[1][1] * M[3][0];
        T A0112 = M[1][0] * M[2][1] - M[1][1] * M[2][0];

        T Result[4][4];

        Result[0][0] = Determinant * (M[1][1] * A2323 - M[1][2] * A1323 + M[1][3] * A1223);
        Result[0][1] = Determinant * -(M[0][1] * A2323 - M[0][2] * A1323 + M[0][3] * A1223);
        Result[0][2] = Determinant * (M[0][1] * A2313 - M[0][2] * A1313 + M[0][3] * A1213);
        Result[0][3] = Determinant * -(M[0][1] * A2312 - M[0][2] * A1312 + M[0][3] * A1212);
        Result[1][0] = Determinant * -(M[1][0] * A2323 - M[1][2] * A0323 + M[1][3] * A0223);
        Result[1][1] = Determinant * (M[0][0] * A2323 - M[0][2] * A0323 + M[0][3] * A0223);
        Result[1][2] = Determinant * -(M[0][0] * A2313 - M[0][2] * A0313 + M[0][3] * A0213);
        Result[1][3] = Determinant * (M[0][0] * A2312 - M[0][2] * A0312 + M[0][3] * A0212);
        Result[2][0] = Determinant * (M[1][0] * A1323 - M[1][1] * A0323 + M[1][3] * A0123);
        Result[2][1] = Determinant * -(M[0][0] * A1323 - M[0][1] * A0323 + M[0][3] * A0123);
        Result[2][2] = Determinant * (M[0][0] * A1313 - M[0][1] * A0313 + M[0][3] * A0113);
        Result[2][3] = Determinant * -(M[0][0] * A1312 - M[0][1] * A0312 + M[0][3] * A0112);
        Result[3][0] = Determinant * -(M[1][0] * A1223 - M[1][1] * A0223 + M[1][2] * A0123);
        Result[3][1] = Determinant * (M[0][0] * A1223 - M[0][1] * A0223 + M[0][2] * A0123);
        Result[3][2] = Determinant * -(M[0][0] * A1213 - M[0][1] * A0213 + M[0][2] * A0113);
        Result[3][3] = Determinant * (M[0][0] * A1212 - M[0][1] * A0212 + M[0][2] * A0112);

        TMatrix Out;
        std::memcpy(Out.M, &Result, 16 * sizeof(T));
        Out.CheckNaN();
        return Out;
    }

    TVector3<T> GetScaledAxis(const EAxis InAxis) const
    {
        switch (InAxis)
        {
        case EAxis::X :
            {
                return TVector3(M[0][0], M[1][0], M[2][0]);
            }
        case EAxis::Y :
            {
                return TVector3(M[0][1], M[1][1], M[2][1]);
            }
        case EAxis::Z :
            {
                return TVector3(M[0][2], M[1][2], M[2][2]);
            }
        }
        return TVector3<T>::ZeroVector();
    }

    TRotator<T> GetRotator() const
    {
        const TVector3 XAxis = GetScaledAxis(EAxis::X);
        const TVector3 YAxis = GetScaledAxis(EAxis::Y);
        const TVector3 ZAxis = GetScaledAxis(EAxis::Z);
        const T RadToDeg = 180.0f / (P_PI * 2.0f);

        T Pitch = Math::ATan2(XAxis.Z, Math::Sqrt(Math::Square(XAxis.X) + Math::Square(XAxis.Y))) * RadToDeg;
        T Yaw = Math::ATan2(XAxis.Y, XAxis.X) * RadToDeg;
        TRotator Rotator = TRotator(Pitch, Yaw, T(0));

        const TVector3 SYAxis = MakeRotationMatrix(Rotator).GetScaledAxis(EAxis::Y);
        Rotator.Roll = Math::ATan2(Math::Dot(ZAxis, SYAxis), Math::Dot(YAxis, SYAxis)) * RadToDeg;

        return Rotator;
    }
    TVector3<T> GetScale(T Tolerance = 0.00000001f);
    TVector3<T> GetTranslation()
    {
        return TVector3(M[3][0], M[3][1], M[3][2]);
    }

    static TMatrix PerspectiveMatrix(T HalfFov, T Width, T Height, T MinZ)
    {
        return TMatrix{
            TPlane<T>(1.0f / Math::Tan(HalfFov), 0.0f, 0.0f, 0.0f),
            TPlane<T>(0.0f, Width / Math::Tan(HalfFov) / Height, 0.0f, 0.0f),
            TPlane<T>(0.0f, 0.0f, 0.0f, 1.0f),
            TPlane<T>(0.0f, 0.0f, MinZ, 0.0f)
        };
    }

    static TMatrix MakeFromX(TVector3<T> const& XAxis)
    {
        TVector3<T> const NewX = XAxis.GetSafeNormal();

        // try to use up if possible
        TVector3<T> const UpVector = (Math::Abs(NewX.Z) < (1.0f - P_SMALL_NUMBER)) ? TVector3<T>(0, 0, 1.0f) : TVector3<T>(1.0f, 0, 0);

        const TVector3<T> NewY = Math::Cross(UpVector, NewX).GetSafeNormal();
        const TVector3<T> NewZ = Math::Cross(NewX, NewY);

        return TMatrix<T>(NewX, NewY, NewZ, TVector3<T>::ZeroVector());
    }

    static TMatrix MakeFromX(T Angle)
    {
        T C = Math::Cos(Angle);
        T S = Math::Sin(Angle);
        return TMatrix(
            TPlane<T>(1, 0, 0, 0),
            TPlane<T>(0, C, -S, 0),
            TPlane<T>(0, S, C, 0),
            TPlane<T>(0, 0, 0, 1)
        );
    }

    static TMatrix MakeFromY(TVector3<T> const& YAxis)
    {
        TVector3<T> const NewY = YAxis.GetSafeNormal();

        // try to use up if possible
        TVector3<T> const UpVector = (Math::Abs(NewY.Z) < (1.0f - P_SMALL_NUMBER)) ? TVector3<T>(0, 0, 1.0f) : TVector3<T>(1.0f, 0, 0);

        const TVector3<T> NewZ = Math::Cross(UpVector, NewY).GetSafeNormal();
        const TVector3<T> NewX = Math::Cross(NewY, NewZ);

        return TMatrix<T>(NewX, NewY, NewZ, TVector3<T>::ZeroVector());
    }

    static TMatrix MakeFromY(T Angle)
    {
        T C = Math::Cos(Angle);
        T S = Math::Sin(Angle);
        return TMatrix(
            TPlane<T>(C, 0, S, 0),
            TPlane<T>(0, 1, 0, 0),
            TPlane<T>(-S, 0, C, 0),
            TPlane<T>(0, 0, 0, 1)
        );
    }

    static TMatrix MakeFromZ(TVector3<T> const& ZAxis)
    {
        TVector3<T> const NewZ = ZAxis.GetSafeNormal();

        // try to use up if possible
        TVector3<T> const UpVector = (Math::Abs(NewZ.Z) < (1.0f - P_SMALL_NUMBER)) ? TVector3<T>(0, 0, 1.0f) : TVector3<T>(1.0f, 0, 0);

        const TVector3<T> NewX = Math::Cross(UpVector, NewZ).GetSafeNormal();
        const TVector3<T> NewY = Math::Cross(NewZ, NewX);

        return TMatrix(NewX, NewY, NewZ, TVector3<T>::ZeroVector());
    }

    static TMatrix MakeFromZ(T Angle)
    {
        T C = Math::Cos(Angle);
        T S = Math::Sin(Angle);
        return TMatrix(
            TPlane<T>(C, -S, 0, 0),
            TPlane<T>(S, C, 0, 0),
            TPlane<T>(0, 0, 1, 0),
            TPlane<T>(0, 0, 0, 1)
        );
    }

    T Get(int32 X, int32 Y) const
    {
        return M[X][Y];
    }

    void Set(int32 X, int32 Y, T Value)
    {
        M[X][Y] = Value;
    }

    std::string ToString() const
    {
        std::string Output;

        Output += std::format("[{} {} {} {}] ", M[0][0], M[0][1], M[0][2], M[0][3]);
        Output += std::format("[{} {} {} {}] ", M[1][0], M[1][1], M[1][2], M[1][3]);
        Output += std::format("[{} {} {} {}] ", M[2][0], M[2][1], M[2][2], M[2][3]);
        Output += std::format("[{} {} {} {}] ", M[3][0], M[3][1], M[3][2], M[3][3]);

        return Output;
    }

    TMatrix operator+(const TMatrix& Other) const
    {
        TMatrix Result;
        for (int32 X = 0; X < 4; X++)
        {
            for (int32 Y = 0; Y < 4; Y++)
            {
                Result.M[X][Y] = M[X][Y] + Other.M[X][Y];
            }
        }
        return Result;
    }
    TMatrix& operator+=(const TMatrix& Other)
    {
        *this = *this + Other;
        CheckNaN();
        return *this;
    }
    TMatrix operator-(const TMatrix& Other) const
    {
        TMatrix Result;
        for (int32 X = 0; X < 4; X++)
        {
            for (int32 Y = 0; Y < 4; Y++)
            {
                Result.M[X][Y] = M[X][Y] - Other.M[X][Y];
            }
        }
        return Result;
    }
    TMatrix& operator-=(const TMatrix& Other)
    {
        *this = *this - Other;
        CheckNaN();
        return *this;
    }
    TMatrix operator*(const TMatrix& Other) const
    {
        TMatrix Result;
        for (uint32 X = 0; X < 4; X++)
        {
            for (uint32 Y = 0; Y < 4; Y++)
            {
                T Num = 0;
                for (uint32 Z = 0; Z < 4; Z++)
                {
                    Num += M[X][Y] * Other.M[Z][Y];
                }
                Result.M[X][Y] = Num;
            }
        }
        return Result;
    }
    TMatrix& operator*=(const TMatrix& Other)
    {
        *this = *this * Other;
        CheckNaN();
        return *this;
    }
    TMatrix operator/(const TMatrix& Other) const
    {
        TMatrix Result;
        for (int32 X = 0; X < 4; X++)
        {
            for (int32 Y = 0; Y < 4; Y++)
            {
                Result.M[X][Y] = M[X][Y] / Other.M[X][Y];
            }
        }
        return Result;
    }
    TMatrix& operator/=(const TMatrix& Other)
    {
        *this = *this / Other;
        CheckNaN();
        return *this;
    }

    TVector3<T> operator*(const TVector3<T>& V3) const
    {
        T TempX = V3.X * M[0][0] + V3.Y * M[1][0] + V3.Z * M[2][0];
        T TempY = V3.X * M[0][1] + V3.Y * M[1][1] + V3.Z * M[2][1];
        T TempZ = V3.X * M[0][2] + V3.Y * M[1][2] + V3.Z * M[2][2];

        return {TempX, TempY, TempZ};
    }

    TVector4<T> operator*(const TVector4<T>& V4) const
    {
        T TempX = V4.X * M[0][0] + V4.Y * M[1][0] + V4.Z * M[2][0] + V4.W * M[3][0];
        T TempY = V4.X * M[0][1] + V4.Y * M[1][1] + V4.Z * M[2][1] + V4.W * M[3][1];
        T TempZ = V4.X * M[0][2] + V4.Y * M[1][2] + V4.Z * M[2][2] + V4.W * M[3][2];
        T TempW = V4.X * M[0][3] + V4.Y * M[1][3] + V4.Z * M[2][3] + V4.W * M[3][3];

        return {TempX, TempY, TempZ, TempW};
    }

    TMatrix& operator=(const TMatrix& Other) // NOLINT
    {
        std::memcpy(M, &Other.M, 16 * sizeof(T));
        CheckNaN();
        return *this;
    }

    bool Equals(const TMatrix& Other)
    {
        for (int32 X = 0; X < 4; X++)
        {
            for (int32 Y = 0; Y < 4; Y++)
            {
                if (M[X][Y] != Other.M[X][Y])
                {
                    return false;
                }
            }
        }
        return true;
    }
};


template <typename T>
struct TPerspectiveMatrix : TMatrix<T>
{
    TPerspectiveMatrix(T Scale, T Aspect, T MinZ, T MaxZ = P_MAX_Z)
        : TMatrix<T>(
            TPlane<T>(Scale / Aspect, 0.0f, 0.0f, 0.0f),
            TPlane<T>(0.0f, Scale, 0.0f, 0.0f),
            TPlane<T>(0.0f, 0.0f, MaxZ / (MaxZ - MinZ), 1.0f),
            TPlane<T>(0.0f, 0.0f, -(MaxZ * MinZ) / (MaxZ - MinZ), 0.0f))

    {
        //     TPerspectiveMatrix(T Scale, T Aspect, T MinZ, T MaxZ = P_MAX_Z)
        //     {
        //         T Right = Aspect * Scale;
        //         T Left = -Right;
        //         T Top = Scale;
        //         T Bottom = -Top;
        //
        //         this->M[0][0] = 2.0f * MinZ / (Right - Left);
        //         this->M[1][1] = 2.0f * MinZ / (Top - Bottom);
        //
        //         this->M[2][0] = (Right + Left) / (Right - Left);
        //         this->M[2][1] = (Top + Bottom) / (Top - Bottom);
        //         this->M[2][2] = -(MaxZ + MinZ) / (MinZ - MaxZ);
        //         this->M[2][3] = -1.0f; // Left or right handed
        //
        //         this->M[3][2] = -2.0f * MinZ * MaxZ / (MinZ - MaxZ);
        //         this->M[3][3] = 0.0f;
    }
};

template <typename T>
struct TLookAtMatrix : TMatrix<T>
{
    TLookAtMatrix(const TVector3<T>& EyePosition, const TVector3<T>& LookAtPosition, const TVector3<T>& UpVector)
    {
        const TVector3<T> ZAxis = (LookAtPosition - EyePosition).Normalized();
        const TVector3<T> XAxis = (Math::Cross(UpVector, ZAxis)).Normalized();
        const TVector3<T> YAxis = Math::Cross(ZAxis, XAxis).Normalized();

        this->SetIdentity();
        for (uint32 RowIndex = 0; RowIndex < 3; RowIndex++)
        {
            this->M[RowIndex][0] = (&XAxis.X)[RowIndex];
            this->M[RowIndex][1] = (&YAxis.X)[RowIndex];
            this->M[RowIndex][2] = (&ZAxis.X)[RowIndex];
            this->M[RowIndex][3] = 0.0f;
        }

        FVector3 InvEyePosition = EyePosition * -1.0f;
        this->M[3][0] = InvEyePosition.X;
        this->M[3][1] = InvEyePosition.Y;
        this->M[3][2] = InvEyePosition.Z;
        this->M[3][3] = 1.0f;
    }
};

template <typename T>
struct TTranslationMatrix : TMatrix<T>
{
    TTranslationMatrix(const TVector3<T>& Delta)
        : TMatrix<T>(
            TPlane<T>(1, 0, 0, 0),
            TPlane<T>(0, 1, 0, 0),
            TPlane<T>(0, 0, 1, 0),
            TPlane<T>(Delta.X, Delta.Y, Delta.Z, 1)
        )
    {
    }
};

template <typename T>
struct TInverseRotationMatrix : TMatrix<T>
{
    TInverseRotationMatrix(const TRotator<T>& Rot)
        : TMatrix<T>(
            TMatrix<T>( // Yaw
                TPlane<T>(+Math::Cos(Rot.Yaw * P_PI / 180.f), -Math::Sin(Rot.Yaw * P_PI / 180.f), 0.0f, 0.0f),
                TPlane<T>(+Math::Sin(Rot.Yaw * P_PI / 180.f), +Math::Cos(Rot.Yaw * P_PI / 180.f), 0.0f, 0.0f),
                TPlane<T>(0.0f, 0.0f, 1.0f, 0.0f),
                TPlane<T>(0.0f, 0.0f, 0.0f, 1.0f)) *
            TMatrix<T>( // Pitch
                TPlane<T>(+Math::Cos(Rot.Pitch * P_PI / 180.f), 0.0f, -Math::Sin(Rot.Pitch * P_PI / 180.f), 0.0f),
                TPlane<T>(0.0f, 1.0f, 0.0f, 0.0f),
                TPlane<T>(+Math::Sin(Rot.Pitch * P_PI / 180.f), 0.0f, +Math::Cos(Rot.Pitch * P_PI / 180.f), 0.0f),
                TPlane<T>(0.0f, 0.0f, 0.0f, 1.0f)) *
            TMatrix<T>( // Roll
                TPlane<T>(1.0f, 0.0f, 0.0f, 0.0f),
                TPlane<T>(0.0f, +Math::Cos(Rot.Roll * P_PI / 180.f), +Math::Sin(Rot.Roll * P_PI / 180.f), 0.0f),
                TPlane<T>(0.0f, -Math::Sin(Rot.Roll * P_PI / 180.f), +Math::Cos(Rot.Roll * P_PI / 180.f), 0.0f),
                TPlane<T>(0.0f, 0.0f, 0.0f, 1.0f))
        )
    {
    }
};
