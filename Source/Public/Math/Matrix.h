#pragma once

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

    TMatrix(const TVector4<T>& InX, const TVector4<T>& InY, const TVector4<T>& InZ, const TVector4<T>& InW)
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

    TMatrix Flip() const
    {
        TMatrix Out;

        Out.M[0][0] = M[0][0];
        Out.M[0][1] = M[1][0];
        Out.M[0][2] = M[2][0];
        Out.M[0][3] = M[3][0];
        Out.M[1][0] = M[0][1];
        Out.M[1][1] = M[1][1];
        Out.M[1][2] = M[2][1];
        Out.M[1][3] = M[3][1];
        Out.M[2][0] = M[0][2];
        Out.M[2][1] = M[1][2];
        Out.M[2][2] = M[2][2];
        Out.M[2][3] = M[3][2];
        Out.M[3][0] = M[0][3];
        Out.M[3][1] = M[1][3];
        Out.M[3][2] = M[2][3];
        Out.M[3][3] = M[3][3];

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

    TVector3<T> GetAxis(const EAxis InAxis) const
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
        const TVector3 XAxis = GetAxis(EAxis::X);
        const TVector3 YAxis = GetAxis(EAxis::Y);
        const TVector3 ZAxis = GetAxis(EAxis::Z);
        const T RadToDeg = 180.0f / (P_PI * 2.0f);

        T Pitch = Math::ATan2(XAxis.Z, Math::Sqrt(Math::Square(XAxis.X) + Math::Square(XAxis.Y))) * RadToDeg;
        T Yaw = Math::ATan2(XAxis.Y, XAxis.X) * RadToDeg;
        TRotator Rotator = TRotator(Pitch, Yaw, T(0));

        const TVector3 SYAxis = TRotationMatrix<T>(Rotator).GetAxis(EAxis::Y);
        Rotator.Roll = Math::ATan2(Math::Dot(ZAxis, SYAxis), Math::Dot(YAxis, SYAxis)) * RadToDeg;

        return Rotator;
    }
    TVector3<T> GetScale(T Tolerance = 0.00000001f);
    TVector3<T> GetTranslation()
    {
        return TVector3(M[3][0], M[3][1], M[3][2]);
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

    TVector4<T> GetRow(int32 Row) const
    {
        return {M[Row][0], M[Row][1], M[Row][2], M[Row][3]};
    }

    TVector4<T> GetColumn(int32 Column) const
    {
        return {M[0][Column], M[1][Column], M[2][Column], M[3][Column]};
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

    TVector3<T> operator*(const TVector3<T>& V) const
    {
        TVector3<T> Result;
        for (int32 Index = 0; Index < 3; Index++)
        {
            TVector4<T> RowVector({M[Index][0], M[Index][1], M[Index][2], M[Index][3]});
            Result[Index] = Math::Dot(RowVector, TVector4<T>(V));
        }
        return Result;
    }

    TVector4<T> operator*(const TVector4<T>& V) const
    {
        T TempX = V.X * M[0][0] + V.Y * M[1][0] + V.Z * M[2][0] + V.W * M[3][0];
        T TempY = V.X * M[0][1] + V.Y * M[1][1] + V.Z * M[2][1] + V.W * M[3][1];
        T TempZ = V.X * M[0][2] + V.Y * M[1][2] + V.Z * M[2][2] + V.W * M[3][2];
        T TempW = V.X * M[0][3] + V.Y * M[1][3] + V.Z * M[2][3] + V.W * M[3][3];

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
constexpr TMatrix<T> operator*(const TMatrix<T>& M0, const TMatrix<T>& M1)
{
    auto A0 = M0.GetRow(0);
    auto A1 = M0.GetRow(1);
    auto A2 = M0.GetRow(2);
    auto A3 = M0.GetRow(3);

    auto B0 = M1.GetRow(0);
    auto B1 = M1.GetRow(1);
    auto B2 = M1.GetRow(2);
    auto B3 = M1.GetRow(3);

    auto C0 = A0 * B0[0] + A1 * B0[1] + A2 * B0[2] + A3 * B0[3];
    auto C1 = A0 * B1[0] + A1 * B1[1] + A2 * B1[2] + A3 * B1[3];
    auto C2 = A0 * B2[0] + A1 * B2[1] + A2 * B2[2] + A3 * B2[3];
    auto C3 = A0 * B3[0] + A1 * B3[1] + A2 * B3[2] + A3 * B3[3];

    return {C0, C1, C2, C3};
}

template <typename T>
struct TPerspectiveMatrix : TMatrix<T>
{
    TPerspectiveMatrix(T Scale, T Aspect, T MinZ, T MaxZ = P_MAX_Z) : TMatrix<T>()
    {
        this->M[0][0] = T(1) / (Aspect * Scale);
        this->M[1][1] = T(1) / Scale;
        this->M[2][2] = (MaxZ + MinZ) / (MaxZ - MinZ);
        this->M[2][3] = T(1);
        this->M[3][2] = (T(2) * MaxZ * MinZ) / (MaxZ - MinZ);
        this->M[3][3] = T(0);
    }
};

template <typename T>
struct TLookAtMatrix : TMatrix<T>
{
    TLookAtMatrix(const TVector3<T>& EyePosition, const TVector3<T>& LookAtPosition, const TVector3<T>& UpVector) : TMatrix<T>()
    {
        // Forward vector
        const TVector3<T> Forward = (LookAtPosition - EyePosition).Normalized();

        // Right vector
        const TVector3<T> Right = (Math::Cross(Forward, UpVector)).Normalized();

        // Up vector
        const TVector3<T> Up = Math::Cross(Right, Forward);

        //  Rx |  Ux | -Fx | 0
        //  Ry |  Uy | -Fy | 0
        //  Rz |  Uz | -Fz | 0
        // -Tx | -Ty | -Tz | 1
        this->M[0][0] = Right[0];
        this->M[1][0] = Right[1];
        this->M[2][0] = Right[2];
        this->M[3][0] = Math::Dot(Right, -EyePosition);
        this->M[0][1] = Up[0];
        this->M[1][1] = Up[1];
        this->M[2][1] = Up[2];
        this->M[3][1] = Math::Dot(Up, -EyePosition);
        this->M[0][2] = -Forward[0];
        this->M[1][2] = -Forward[1];
        this->M[2][2] = -Forward[2];
        this->M[3][2] = Math::Dot(Forward, -EyePosition);
        
        this->M[3][3] = 1.0f;
    }
};

template <typename T>
struct TTranslationMatrix : TMatrix<T>
{
    TTranslationMatrix(const TVector3<T>& Delta) : TMatrix<T>()
    {
        this->M[3][0] = Delta.X;
        this->M[3][1] = Delta.Y;
        this->M[3][2] = Delta.Z;
    }
};

template <typename T>
struct TRotationMatrix : TMatrix<T>
{
    TRotationMatrix(T Pitch, T Yaw, T Roll) : TMatrix<T>()
    {
        // Convert from degrees to radians
        Pitch = Math::DegreesToRadians(Pitch);
        Yaw = Math::DegreesToRadians(Yaw);
        Roll = Math::DegreesToRadians(Roll);
        
        T CP = Math::Cos(Pitch);
        T SP = Math::Sin(Pitch);
        T CY = Math::Cos(Yaw);
        T SY = Math::Sin(Yaw);
        T CR = Math::Cos(Roll);
        T SR = Math::Sin(Roll);

        T CPSY = CP * SY;
        T SPSY = SP * SY;
        
        this->M[0][0] = CY * CR;
        this->M[0][1] = -CY * SR;
        this->M[0][2] = SY;
        this->M[1][0] = SPSY * CR + CP * SR;
        this->M[1][1] = -SPSY * SR + CP * CR;
        this->M[1][2] = -SP * CY;
        this->M[2][0] = -CPSY * CR + SP * SR;
        this->M[2][1] = CPSY * SR + SP * CR;
        this->M[2][2] = CP * CY;
    }

    TRotationMatrix(const TRotator<T>& Rotation) : TMatrix<T>()
    {
        *this = TRotationMatrix(Rotation.Pitch, Rotation.Yaw, Rotation.Roll);
    }
};

template <typename T>
struct TRotationTranslationMatrix : TMatrix<T>
{
    TRotationTranslationMatrix(const TRotator<T>& Rotation, const TVector3<T>& Translation) : TMatrix<T>()
    {
        TMatrix<T> RotationMatrix = TRotationMatrix<T>(Rotation);
        TMatrix<T> TranslationMatrix = TTranslationMatrix<T>(Translation);
        *this = RotationMatrix * TranslationMatrix;
    }
};