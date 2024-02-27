#pragma once

#include <string>
#include "Types.h"

template <typename T, int N>
struct TMatrix;

typedef TMatrix<float, 2> Matrix2;
typedef TMatrix<float, 3> Matrix3;
typedef TMatrix<float, 4> Matrix4;

template <typename T, int N>
struct TMatrix
{
    static_assert(std::is_floating_point_v<T>, "Type is not floating point.");
    static_assert(2 <= N <= 4, "Matrix can only be size 2, 3, or 4");
    
    T M[N][N];

    TMatrix()
    {
        for (int32 X = 0; X < N; X++)
        {
            for (int32 Y = 0; Y < N; Y++)
            {
                M[X][Y] = 0;
            }
        }
    }
    TMatrix(T Values[N][N])
    {
        M = Values;
    }
    TMatrix(const std::initializer_list<T>& Values)
    {
        for (int32 X = 0; X < N; X++)
        {
            for (int32 Y = 0; Y < N; Y++)
            {
                int32 Index = (X * N) + Y;
                M[X][Y] = *(Values.begin() + Index);
            }
        }
    }

    static TMatrix Identity()
    {
        TMatrix Mat;
        for (int32 X = 0; X < N; X++)
        {
            for (int32 Y = 0; Y < N; Y++)
            {
                Mat.M[X][Y] = X == Y ? 1.0f : 0.0f;
            }
        }
        return Mat;
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

        for (int32 X = 0; X < N; X++)
        {
            Out += "[";
            for (int32 Y = 0; Y < N; Y++)
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
        for (int32 X = 0; X < N; X++)
        {
            for (int32 Y = 0; Y < N; Y++)
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
        for (int32 X = 0; X < N; X++)
        {
            for (int32 Y = 0; Y < N; Y++)
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
        for (int32 X = 0; X < N; X++)
        {
            for (int32 Y = 0; Y < N; Y++)
            {
                Result.M[X][Y] = M[X][Y] * Other;
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
        for (int32 X = 0; X < N; X++)
        {
            for (int32 Y = 0; Y < N; Y++)
            {
                Result.M[X][Y] = M[X][Y] / Other;
            }
        }
        return Result;
    }
    TMatrix& operator/=(const TMatrix& Other)
    {
        *this = *this / Other;
        return *this;
    }
    
    T operator[](int32 Index) const
    {
        int32 X = Index / N;
        int32 Y = Index % N;
        return M[X][Y];
    }

    T& operator[](int32 Index)
    {
        int32 X = Index / N;
        int32 Y = Index % N;
        return M[X][Y];
    }
};
