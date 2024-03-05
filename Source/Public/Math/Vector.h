#pragma once

#include <format>
#include <cassert>

#include "Framework/Core/Logging.h"
#include "Math.h"


// Alias for 'float[4]'
struct alignas(4) FVectorRegister
{
    float V[4];
};

template <typename T>
struct TVector2
{
    static_assert(std::is_arithmetic_v<T>, "Type is not a number.");

    // Memory aligned coordinate values
    union
    {
        struct // NOLINT(clang-diagnostic-nested-anon-types)
        {
            T X;
            T Y;
        };
        T XY[2];
    };

    // Constructors
    TVector2() : X(0), Y(0)
    {
        CheckNaN();
    }
    TVector2(T InX) : X(InX), Y(InX)
    {
        CheckNaN();
    }
    TVector2(T InX, T InY) : X(InX), Y(InY)
    {
        CheckNaN();
    }
    TVector2(const std::initializer_list<T>& Values)
    {
        X = *(Values.begin());
        Y = *(Values.begin() + 1);
        CheckNaN();
    }

    // Functions
    void CheckNaN() const
    {
        if (!(Math::IsFinite(X) && Math::IsFinite(Y)))
        {
            LOG_ERROR("Vector [{}, {}] contains NaN", X, Y)
        }
    }
    static TVector2 ZeroVector() { return TVector2(); }
    static TVector2 IdentityVector() { return TVector2(1); }
    void Normalize()
    {
        X = 1.0f / X;
        Y = 1.0f / Y;
        CheckNaN();
    }
    TVector2 Normalized() const { return {1.0f / X, 1.0f / Y}; }

    std::string ToString() const { return std::format("[{}, {}]", X, Y); }

    // Operators
    TVector2 operator+(const TVector2& V) const { return {X + V.X, Y + V.Y}; }
    TVector2& operator +=(const TVector2& V)
    {
        X += V.X;
        Y += V.Y;
        CheckNaN();
        return *this;
    }
    TVector2 operator-(const TVector2& V) const { return {X - V.X, Y - V.Y}; }
    TVector2& operator -=(const TVector2& V)
    {
        X -= V.X;
        Y -= V.Y;
        CheckNaN();
        return *this;
    }
    TVector2 operator*(const TVector2& V) const { return {X * V.X, Y * V.Y}; }
    TVector2& operator *=(const TVector2& V)
    {
        X *= V.X;
        Y *= V.Y;
        CheckNaN();
        return *this;
    }
    TVector2 operator/(const TVector2& V) const { return {X / V.X, Y / V.Y}; }
    TVector2& operator /=(const TVector2& V)
    {
        X /= V.X;
        Y /= V.Y;
        CheckNaN();
        return *this;
    }
    TVector2 operator-()
    {
        return TVector2(-X, -Y);
    }
    bool operator<(const TVector2& Other)
    {
        return X < Other.X && Y < Other.Y;
    }
    bool operator>(const TVector2& Other)
    {
        return X > Other.X && Y > Other.Y;
    }

    T operator[](int32 Index) const { return XY[Index]; }
    T& operator[](int32 Index) { return XY[Index]; }
};

template <typename T>
struct TVector3
{
    static_assert(std::is_arithmetic_v<T>, "Type is not a number.");

    // Memory aligned coordinate values
    union
    {
        struct // NOLINT(clang-diagnostic-nested-anon-types)
        {
            T X;
            T Y;
            T Z;
        };
        T XYZ[3];
    };

    // Constructors
    TVector3() : X(0), Y(0), Z(0)
    {
        CheckNaN();
    }
    TVector3(T InX) : X(InX), Y(InX), Z(InX)
    {
        CheckNaN();
    }
    TVector3(T InX, T InY, T InZ) : X(InX), Y(InY), Z(InZ)
    {
        CheckNaN();
    }
    TVector3(const std::initializer_list<T>& Values)
    {
        X = *(Values.begin());
        Y = *(Values.begin() + 1);
        Z = *(Values.begin() + 2);
        CheckNaN();
    }


    // Functions
    static TVector3 ZeroVector() { return TVector3(); }
    static TVector3 IdentityVector() { return TVector3(1); }
    static TVector3 ForwardVector() { return TVector3(1, 0, 0); }
    static TVector3 RightVector() { return TVector3(0, 1, 0); }
    static TVector3 UpVector() { return TVector3(0, 0, 1); }
    void CheckNaN() const
    {
        if (!(Math::IsFinite(X) && Math::IsFinite(Y) && Math::IsFinite(Z)))
        {
            LOG_ERROR("Vector [{}, {}, {}] contains NaN", X, Y, Z)
        }
    }

    void Normalize()
    {
        const T SquareSum = X * X + Y * Y + Z * Z;
        if (SquareSum > 0.00001f)
        {
            const T Scale = Math::InvSqrt(SquareSum);
            X *= Scale;
            Y *= Scale;
            Z *= Scale;
        }
        CheckNaN();
    }
    TVector3 Normalized() const
    {
        TVector3 Out(X, Y, Z);
        Out.Normalize();
        return Out;
    }
    TVector3 GetSafeNormal() const
    {
        const T SquareSum = X * X + Y * Y + Z * Z;

        if (SquareSum == 1.f)
        {
            return *this;
        }

        if (SquareSum < 0.000001f)
        {
            return ZeroVector();
        }
        const T Scale = static_cast<T>(Math::InvSqrt(SquareSum));
        return TVector3(X * Scale, Y * Scale, Z * Scale);
    }
    constexpr T Length() const
    {
        return Math::Sqrt(X * X + Y * Y + Z * Z);;
    }
    TVector3 Cross(const TVector3& V) const
    {
        return TVector3{
            Y * V.Z - Z * V.Y,
            X * V.Z - Z * V.X,
            X * V.Y - Y * V.X
        };
    }
    T Dot(const TVector3& V) const
    {
        T Result = 0;
        for (int32 Index = 0; Index < 3; Index++)
        {
            Result += XYZ[Index] * V[Index];
        }
        return Result;
    }
    T Size() const
    {
        return Math::Sqrt(Math::Square(X) + Math::Square(Y) + Math::Square(Z));
    }

    std::string ToString() const { return std::format("[{}, {}, {}]", X, Y, Z); }

    // Operators
    TVector3 operator+(const TVector3& V) const { return {X + V.X, Y + V.Y, Z + V.Z}; }
    TVector3& operator +=(const TVector3& V)
    {
        X += V.X;
        Y += V.Y;
        Z += V.Z;
        CheckNaN();
        return *this;
    }
    TVector3 operator-(const TVector3& V) const { return {X - V.X, Y - V.Y, Z - V.Z}; }
    TVector3& operator -=(const TVector3& V)
    {
        X -= V.X;
        Y -= V.Y;
        Z -= V.Z;
        CheckNaN();
        return *this;
    }
    TVector3 operator*(const TVector3& V) const { return {X * V.X, Y * V.Y, Z * V.Z}; }
    TVector3& operator *=(const TVector3& V)
    {
        X *= V.X;
        Y *= V.Y;
        Z *= V.Z;
        CheckNaN();
        return *this;
    }
    TVector3 operator/(const TVector3& V) const { return {X / V.X, Y / V.Y, Z / V.Z}; }
    TVector3& operator /=(const TVector3& V)
    {
        X /= V.X;
        Y /= V.Y;
        Z /= V.Z;
        CheckNaN();
        return *this;
    }
    bool operator==(const TVector3& V) const { return X == V.X && Y == V.Y && Z == V.Z; }
    bool operator!=(const TVector3& V) const { return X != V.X || Y != V.Y || Z != V.Z; }
    TVector3 operator-()
    {
        return TVector3(-X, -Y, -Z);
    }
    bool operator<(const TVector3& Other)
    {
        return X < Other.X && Y < Other.Y && Z < Other.Z;
    }
    bool operator>(const TVector3& Other)
    {
        return X > Other.X && Y > Other.Y && Z > Other.Z;
    }

    T operator[](int32 Index) const { return XYZ[Index]; }
    T& operator[](int32 Index) { return XYZ[Index]; }
};

template <typename T>
struct TVector4
{
    static_assert(std::is_arithmetic_v<T>, "Type is not a number.");

    // Memory aligned coordinate values
    union
    {
        struct // NOLINT(clang-diagnostic-nested-anon-types)
        {
            T X;
            T Y;
            T Z;
            T W;
        };
        T XYZW[4];
    };

    // Constructors
    TVector4() : X(0), Y(0), Z(0), W(0)
    {
        CheckNaN();
    }
    TVector4(T InX) : X(InX), Y(InX), Z(InX), W(InX)
    {
        CheckNaN();
    }
    TVector4(T InX, T InY, T InZ, T InW) : X(InX), Y(InY), Z(InZ), W(InW)
    {
        CheckNaN();
    }
    TVector4(const std::initializer_list<T>& Values)
    {
        X = *(Values.begin());
        Y = *(Values.begin() + 1);
        Z = *(Values.begin() + 2);
        W = *(Values.begin() + 3);
        CheckNaN();
    }
    TVector4(const TVector3<T>& V, T InW) : X(V.X), Y(V.Y), Z(V.Z), W(InW)
    {
        CheckNaN();
    }

    // Functions
    static TVector4 ZeroVector() { return TVector4(); }
    static TVector4 IdentityVector() { return TVector4(1); }
    void CheckNaN() const
    {
        if (!(Math::IsFinite(X) && Math::IsFinite(Y) && Math::IsFinite(Z) && Math::IsFinite(W)))
        {
            LOG_ERROR("Vector [{}, {}, {}, {}] contains NaN", X, Y, Z, W)
        }
    }
    void Normalize()
    {
        X = T(1.0) / X;
        Y = T(1.0) / Y;
        Z = T(1.0) / Z;
        W = T(1.0) / W;
    }
    TVector4 Normalized() const { return {T(1.0) / X, T(1.0) / Y, T(1.0) / Z, T(1.0) / W}; }

    FVectorRegister Register() const
    {
        FVectorRegister Reg;
        Reg.V[0] = X;
        Reg.V[1] = Y;
        Reg.V[2] = Z;
        Reg.V[3] = W;
        return Reg;
    }

    std::string ToString() const { return std::format("[{}, {}, {}, {}]", X, Y, Z, W); }

    // Operators
    TVector4 operator+(const TVector4& V) const { return {X + V.X, Y + V.Y, Z + V.Z, W + V.W}; }
    TVector4& operator +=(const TVector4& V)
    {
        X += V.X;
        Y += V.Y;
        Z += V.Z;
        W += V.W;
        CheckNaN();
        return *this;
    }
    TVector4 operator-(const TVector4& V) const { return {X - V.X, Y - V.Y, Z - V.Z, W - V.W}; }
    TVector4& operator -=(const TVector4& V)
    {
        X -= V.X;
        Y -= V.Y;
        Z -= V.Z;
        W -= V.W;
        CheckNaN();
        return *this;
    }
    TVector4 operator*(const TVector4& V) const { return {X * V.X, Y * V.Y, Z * V.Z, W * V.W}; }
    TVector4& operator *=(const TVector4& V)
    {
        X *= V.X;
        Y *= V.Y;
        Z *= V.Z;
        W *= V.W;
        CheckNaN();
        return *this;
    }
    TVector4 operator/(const TVector4& V) const { return {X / V.X, Y / V.Y, Z / V.Z, W / V.W}; }
    TVector4& operator /=(const TVector4& V)
    {
        X /= V.X;
        Y /= V.Y;
        Z /= V.Z;
        W /= V.W;
        CheckNaN();
        return *this;
    }
    TVector4 operator-()
    {
        return TVector4(-X, -Y, -Z, -W);
    }
    bool operator<(const TVector4& Other)
    {
        return X < Other.X && Y < Other.Y && Z < Other.Z;
    }
    bool operator>(const TVector4& Other)
    {
        return X > Other.X && Y > Other.Y && Z > Other.Z && W > Other.W;
    }

    T operator[](int32 Index) const { return XYZW[Index]; }
    T& operator[](int32 Index) { return XYZW[Index]; }
};

namespace Math
{
    template <typename T>
    static T Cross(const TVector2<T>& A, const TVector2<T>& B)
    {
        return A.X * B.Y - A.Y * B.X;
    }

    template <typename T>
    static TVector3<T> Cross(const TVector3<T>& A, const TVector3<T>& B)
    {
        return TVector3<T>{
            A.Y * B.Z - A.Z * B.Y,
            A.X * B.Z - A.Z * B.X,
            A.X * B.Y - A.Y * B.X
        };
    }

    template <typename T>
    static T Dot(const TVector3<T>& A, const TVector3<T>& B)
    {
        T Result = 0;
        for (int32 Index = 0; Index < 3; Index++)
        {
            Result += A.XYZ[Index] * B[Index];
        }
        return Result;
    }

    template <typename T>
    static T CrossDot(const TVector3<T>& A, const TVector3<T>& B, const TVector3<T>& P)
    {
        return Math::Dot(Math::Cross(A, B), P);
    }
}


template <typename T>
struct TBarycentric
{
    // Vector Sign
    static bool EdgeSign(const TVector2<T>& A, const TVector2<T>& B, const TVector2<T>& C)
    {
        T Result = (C[0] - A[0]) * (B[1] - A[1]) - (C[1] - A[1]) * (B[0] - A[0]);
        return Math::Sign(Result);
    }

    static float EdgeValue(const TVector2<T>& A, const TVector2<T>& B, const TVector2<T>& P)
    {
        return Math::Cross(B - A, P - A);
    }

    // Muller-Trumbore ray triangle intersect
    static bool GetBarycentric(const TVector3<T>& P,
                               TVector3<T>* Points,
                               TVector3<T>& UVW,
                               T Tolerance = P_VERY_SMALL_NUMBER)
    {
        const TVector3<T> A = Points[0];
        const TVector3<T> B = Points[1];
        const TVector3<T> C = Points[2];

        const TVector3<T> V0 = B - A;
        const TVector3<T> V1 = C - A;
        const TVector3<T> V2 = P - A;

        const T D00 = Math::Dot(V0, V0);
        const T D01 = Math::Dot(V0, V1);
        const T D11 = Math::Dot(V1, V1);
        const T D20 = Math::Dot(V2, V0);
        const T D21 = Math::Dot(V2, V1);
        const T Denom = 1.0f / (D00 * D11 - D01 * D01);

        const T V = (D11 * D20 - D01 * D21) * Denom;
        const T W = (D00 * D21 - D01 * D20) * Denom;
        const T U = 1.0f - V - W;

        UVW.X = U;
        UVW.Y = V;
        UVW.Z = W;

        T Sum = UVW.X + UVW.Y + UVW.Z;
        T OneMinusSum = 1.0f - Sum;
        return (
            UVW.X > 0.0f &&
            UVW.Y > 0.0f &&
            UVW.Z > 0.0f &&
            (1.0f - Sum) < Tolerance);
    }

    static EWindingOrder GetVertexOrder(const TVector3<T>& P, const TVector3<T>& Q, const TVector3<T>& R)
    {
        const float Result = (Q.Y - P.Y) * (R.X - Q.X) - (Q.X - P.X) * (R.Y - Q.Y);
        if (Result == 0.0f)
        {
            return CL;
        }
        return Result > 0.0f ? CW : CCW;
    }
};
