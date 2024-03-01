#pragma once

#include <string>

#include "Math.h"
#include "Types.h"
#include "Vector.h"
#include "Plane.h"

template <typename T>
struct TMatrix;
typedef TMatrix<float> PMatrix4;

// M[RowIndex][ColumnIndex]
template <typename T>
struct TMatrix
{
    static_assert(std::is_floating_point_v<T>, "Type is not floating point.");

    alignas(16) T M[4][4];

    TMatrix()
    {
        SetIdentity();
    }

    TMatrix(const PPlane& InX, const PPlane& InY, const PPlane& InZ, const PPlane& InW)
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
    }

    bool Equals(const TMatrix& Other, float Threshold = 0.00001f) const
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
        return {
            PPlane(1, 0, 0, 0),
            PPlane(0, 1, 0, 0),
            PPlane(0, 0, 1, 0),
            PPlane(0, 0, 0, 1)
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

    constexpr T GetDeterminant()
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
        return Out;
    }

    static PMatrix4 ProjectionMatrix(float HalfFov, float Width, float Height, float MinZ)
    {
        return {
            PPlane(1.0f / Math::Tan(HalfFov), 0.0f, 0.0f, 0.0f),
            PPlane(0.0f, Width / Math::Tan(HalfFov) / Height, 0.0f, 0.0f),
            PPlane(0.0f, 0.0f, 0.0f, 1.0f),
            PPlane(0.0f, 0.0f, MinZ, 0.0f)
        };
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
        std::string Out;

        for (int32 X = 0; X < 4; X++)
        {
            Out += "[";
            for (int32 Y = 0; Y < 4; Y++)
            {
                Out += std::to_string(M[X][Y]) + ", ";
            }
            Out += "]\n";
        }

        return Out;
    }

    // Operators
    TMatrix operator+(const TMatrix& Other)
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
        return *this;
    }
    TMatrix operator-(const TMatrix& Other)
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
        return *this;
    }
    TMatrix operator*(const TMatrix& Other)
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
        return *this;
    }
    TMatrix operator/(const TMatrix& Other)
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
        return *this;
    }

    PVector4 operator*(const PVector4& V4) const
    {
        T TempX, TempY, TempZ, TempW;

        TempX = V4.X * M[0][0] + V4.Y * M[1][0] + V4.Z * M[2][0] + V4.W * M[3][0];
        TempY = V4.X * M[0][1] + V4.Y * M[1][1] + V4.Z * M[2][1] + V4.W * M[3][1];
        TempZ = V4.X * M[0][2] + V4.Y * M[1][2] + V4.Z * M[2][2] + V4.W * M[3][2];
        TempW = V4.X * M[0][3] + V4.Y * M[1][3] + V4.Z * M[2][3] + V4.W * M[3][3];

        return {TempX, TempY, TempZ, TempW};
    }
};
