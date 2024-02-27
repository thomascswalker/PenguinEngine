#pragma once

#include <stdexcept>
#include <type_traits>
#include <format>

#include "Types.h"

template <typename T> struct TVector2;
template <typename T> struct TVector3;
template <typename T> struct TVector4;

typedef TVector2<float> Vector2;
typedef TVector2<double> Vector2d;
typedef TVector3<float> Vector3;
typedef TVector3<double> Vector3d;
typedef TVector4<float> Vector4;
typedef TVector4<double> Vector4d;

template <typename T>
struct TVector2
{
    static_assert(std::is_floating_point_v<T>, "Type is not floating point.");
    
    // Memory aligned coordinate values
    union
    {
        struct
        {
            T X;
            T Y;
        };
        T XY[2];
    };

    // Constructors
    TVector2() : X(0), Y(0)
    {
        CHECK_FP_TYPE(T)
    }
    TVector2(T InX) : X(InX), Y(InX)
    {
        CHECK_FP_TYPE(T)
    }
    TVector2(T InX, T InY, T InZ) : X(InX), Y(InY)
    {
        CHECK_FP_TYPE(T)
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
    // Memory aligned coordinate values
    union
    {
        struct
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
        CHECK_FP_TYPE(T)
    }
    TVector3(T InX) : X(InX), Y(InX), Z(InX)
    {
        CHECK_FP_TYPE(T)
    }
    TVector3(T InX, T InY, T InZ) : X(InX), Y(InY), Z(InZ)
    {
        CHECK_FP_TYPE(T)
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
        T Result;
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

    T operator[](int32 Index) const { return XYZ[Index]; }
    T& operator[](int32 Index) { return XYZ[Index]; }
};

template <typename T>
struct TVector4
{
    // Memory aligned coordinate values
    union
    {
        struct
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
        CHECK_FP_TYPE(T)
    }
    TVector4(T InX) : X(InX), Y(InX), Z(InX), W(InX)
    {
        CHECK_FP_TYPE(T)
    }
    TVector4(T InX, T InY, T InZ, T InW) : X(InX), Y(InY), Z(InZ), W(InW)
    {
        CHECK_FP_TYPE(T)
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
