#pragma once

#include <format>

#include "Math.h"
#include "Types.h"

template <typename T>
struct TVector2;
template <typename T>
struct TVector3;
template <typename T>
struct TVector4;

typedef TVector2<float> PVector2;
typedef TVector2<double> PVector2d;
typedef TVector3<float> PVector3;
typedef TVector3<double> PVector3d;
typedef TVector4<float> PVector4;
typedef TVector4<double> PVector4d;

template <typename T>
struct TVector2
{
    static_assert(std::is_floating_point_v<T>, "Type is not floating point.");

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
    }
    TVector2(T InX) : X(InX), Y(InX)
    {
    }
    TVector2(T InX, T InY) : X(InX), Y(InY)
    {
    }
    TVector2(const std::initializer_list<T>& Values)
    {
        X = *(Values.begin());
        Y = *(Values.begin() + 1);
    }

    // Functions
    static TVector2 ZeroVector() { return TVector2(); }
    static TVector2 IdentityVector() { return TVector2(1); }

    void Normalize()
    {
        X = T(1.0) / X;
        Y = T(1.0) / Y;
    }
    TVector2 Normalized() const { return {T(1.0) / X, T(1.0) / Y}; }

    constexpr std::string ToString() const { return std::format("[{}, {}]", X, Y); }

    // Operators
    TVector2 operator+(const TVector2& V) const { return {X + V.X, Y + V.Y}; }
    TVector2& operator +=(const TVector2& V)
    {
        X += V.X;
        Y += V.Y;
        return *this;
    }
    TVector2 operator-(const TVector2& V) const { return {X - V.X, Y - V.Y}; }
    TVector2& operator -=(const TVector2& V)
    {
        X -= V.X;
        Y -= V.Y;
        return *this;
    }
    TVector2 operator*(const TVector2& V) const { return {X * V.X, Y * V.Y}; }
    TVector2& operator *=(const TVector2& V)
    {
        X *= V.X;
        Y *= V.Y;
        return *this;
    }
    TVector2 operator/(const TVector2& V) const { return {X / V.X, Y / V.Y}; }
    TVector2& operator /=(const TVector2& V)
    {
        X /= V.X;
        Y /= V.Y;
        return *this;
    }

    T operator[](int32 Index) const { return XY[Index]; }
    T& operator[](int32 Index) { return XY[Index]; }
};

template <typename T>
struct TVector3
{
    static_assert(std::is_floating_point_v<T>, "Type is not floating point.");

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
    }
    TVector3(T InX) : X(InX), Y(InX), Z(InX)
    {
    }
    TVector3(T InX, T InY, T InZ) : X(InX), Y(InY), Z(InZ)
    {
    }
    TVector3(const std::initializer_list<T>& Values)
    {
        X = *(Values.begin());
        Y = *(Values.begin() + 1);
        Z = *(Values.begin() + 2);
    }

    // Functions
    static TVector3 ZeroVector() { return TVector3(); }
    static TVector3 IdentityVector() { return TVector3(1); }

    void Normalize()
    {
        X = T(1.0) / X;
        Y = T(1.0) / Y;
        Z = T(1.0) / Z;
    }
    TVector3 Normalized() const { return {T(1.0) / X, T(1.0) / Y, T(1.0) / Z}; }

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

    constexpr std::string ToString() const { return std::format("[{}, {}, {}]", X, Y, Z); }

    // Operators
    TVector3 operator+(const TVector3& V) const { return {X + V.X, Y + V.Y, Z + V.Z}; }
    TVector3& operator +=(const TVector3& V)
    {
        X += V.X;
        Y += V.Y;
        Z += V.Z;
        return *this;
    }
    TVector3 operator-(const TVector3& V) const { return {X - V.X, Y - V.Y, Z - V.Z}; }
    TVector3& operator -=(const TVector3& V)
    {
        X -= V.X;
        Y -= V.Y;
        Z -= V.Z;
        return *this;
    }
    TVector3 operator*(const TVector3& V) const { return {X * V.X, Y * V.Y, Z * V.Z}; }
    TVector3& operator *=(const TVector3& V)
    {
        X *= V.X;
        Y *= V.Y;
        Z *= V.Z;
        return *this;
    }
    TVector3 operator/(const TVector3& V) const { return {X / V.X, Y / V.Y, Z / V.Z}; }
    TVector3& operator /=(const TVector3& V)
    {
        X /= V.X;
        Y /= V.Y;
        Z /= V.Z;
        return *this;
    }
    bool operator==(const TVector3& V) const { return X == V.X && Y == V.Y && Z == V.Z; }
    bool operator!=(const TVector3& V) const { return X != V.X || Y != V.Y || Z != V.Z; }

    T operator[](int32 Index) const { return XYZ[Index]; }
    T& operator[](int32 Index) { return XYZ[Index]; }
};

template <typename T>
struct TVector4
{
    static_assert(std::is_floating_point_v<T>, "Type is not floating point.");

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
    }
    TVector4(T InX) : X(InX), Y(InX), Z(InX), W(InX)
    {
    }
    TVector4(T InX, T InY, T InZ, T InW) : X(InX), Y(InY), Z(InZ), W(InW)
    {
    }
    TVector4(const std::initializer_list<T>& Values)
    {
        X = *(Values.begin());
        Y = *(Values.begin() + 1);
        Z = *(Values.begin() + 2);
        W = *(Values.begin() + 3);
    }
    TVector4(const PVector3& V, float InW) : X(V.X), Y(V.Y), Z(V.Z), W(InW)
    {
    }

    // Functions
    static TVector4 ZeroVector() { return TVector4(); }
    static TVector4 IdentityVector() { return TVector4(1); }

    void Normalize()
    {
        X = T(1.0) / X;
        Y = T(1.0) / Y;
        Z = T(1.0) / Z;
        W = T(1.0) / W;
    }
    TVector4 Normalized() const { return {T(1.0) / X, T(1.0) / Y, T(1.0) / Z, T(1.0) / W}; }

    constexpr std::string ToString() const { return std::format("[{}, {}, {}, {}]", X, Y, Z, W); }

    // Operators
    TVector4 operator+(const TVector4& V) const { return {X + V.X, Y + V.Y, Z + V.Z, W + V.W}; }
    TVector4& operator +=(const TVector4& V)
    {
        X += V.X;
        Y += V.Y;
        Z += V.Z;
        W += V.W;
        return *this;
    }
    TVector4 operator-(const TVector4& V) const { return {X - V.X, Y - V.Y, Z - V.Z, W - V.W}; }
    TVector4& operator -=(const TVector4& V)
    {
        X -= V.X;
        Y -= V.Y;
        Z -= V.Z;
        W -= V.W;
        return *this;
    }
    TVector4 operator*(const TVector4& V) const { return {X * V.X, Y * V.Y, Z * V.Z, W * V.W}; }
    TVector4& operator *=(const TVector4& V)
    {
        X *= V.X;
        Y *= V.Y;
        Z *= V.Z;
        W *= V.W;
        return *this;
    }
    TVector4 operator/(const TVector4& V) const { return {X / V.X, Y / V.Y, Z / V.Z, W / V.W}; }
    TVector4& operator /=(const TVector4& V)
    {
        X /= V.X;
        Y /= V.Y;
        Z /= V.Z;
        W /= V.W;
        return *this;
    }

    T operator[](int32 Index) const { return XYZW[Index]; }
    T& operator[](int32 Index) { return XYZW[Index]; }
};

namespace Math
{
    static float VectorSign(const PVector2& Vec, const PVector2& A, const PVector2& B)
    {
        return Sign((B.X - A.X) * (Vec.Y - A.Y) - (B.Y - A.Y) * (Vec.X - A.X));
    }

    // Returns true when the point is inside the triangle
    // Should not return true when the point is on one of the edges
    static bool IsPointInTriangle(const PVector2& TestPoint, const PVector2& A, const PVector2& B, const PVector2& C)
    {
        const float BA = VectorSign(B, A, TestPoint);
        const float CB = VectorSign(C, B, TestPoint);
        const float AC = VectorSign(A, C, TestPoint);

        // point is in the same direction of all 3 tri edge lines
        // must be inside, regardless of tri winding
        return BA == CB && CB == AC; // NOLINT
    }

    // Muller-Trumbore ray triangle intersect
    static bool ClosestPointBarycentrics(const PVector3& P, const PVector3& V0, const PVector3& V1, const PVector3& V2, PVector3& UVW)
    {
        const PVector3 Edge01 = V1 - V0;
        const PVector3 Edge02 = V2 - V0;
        const PVector3 Origin = P - V0;
        
        const PVector3 Normal = Edge02.Cross(Edge01);
        const PVector3 Dir = Normal.Cross(Edge02);
        const float InvDet = 1.0f / Edge01.Dot(Dir);
        
        UVW.Y = InvDet * Origin.Dot(Dir);
        UVW.Z = InvDet * Normal.Dot(Origin.Cross(Edge01));
        UVW.X = 1.0f - UVW.Y - UVW.Z;

        return true;
    }
}
