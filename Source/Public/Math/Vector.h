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
    bool operator>(T Value)
    {
        return X > Value && Y > Value;
    }
    bool operator<(T Value)
    {
        return X < Value && Y < Value;
    }
    bool operator ==(T Value)
    {
        return X == Value && Y == Value;
    }
    bool operator ==(const TVector2& Other)
    {
        return X == Other.X && Y == Other.Y;
    }
    bool operator !=(const TVector2& Other)
    {
        return X != Other.X || Y != Other.Y;
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
    TVector3(const TVector2<T>& V, T InZ = T(1)) : X(V.X), Y(V.Y), Z(InZ)
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
    static TVector3 UpVector() { return TVector3(0, 1, 0); }
    static TVector3 RightVector() { return TVector3(0, 0, 1); }
    void CheckNaN() const
    {
        if (!(Math::IsFinite(X) && Math::IsFinite(Y) && Math::IsFinite(Z)))
        {
            LOG_ERROR("Vector [{}, {}, {}] contains NaN", X, Y, Z)
        }
    }

    void Normalize()
    {
        const T Magnitude = Length();
        if (Magnitude < 0.000001f)
        {
            *this = ZeroVector();
        }
        else
        {
            X /= Magnitude;
            Y /= Magnitude;
            Z /= Magnitude;
        }
        CheckNaN();
    }
    TVector3 Normalized() const
    {
        TVector3 Out(X, Y, Z);
        Out.Normalize();
        return Out;
    }

    constexpr T Length() const
    {
        return Math::Sqrt(X * X + Y * Y + Z * Z);
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

    TVector3 operator-() const
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

    operator TVector2<T>() const
    {
        return {X, Y};
    }
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
    TVector4(T* Values) : X(Values[0]), Y(Values[1]), Z(Values[2]), W(Values[3])
    {
    }
    TVector4(const std::initializer_list<T>& Values)
    {
        X = *(Values.begin());
        Y = *(Values.begin() + 1);
        Z = *(Values.begin() + 2);
        W = *(Values.begin() + 3);
        CheckNaN();
    }
    TVector4(const TVector3<T>& V, T InW = T(1)) : X(V.X), Y(V.Y), Z(V.Z), W(InW)
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

    operator TVector2<T>() const
    {
        return {X, Y};
    }

    operator TVector3<T>() const
    {
        return {X, Y, Z};
    }
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
        T Result = T(0);
        for (int32 Index = 0; Index < 3; Index++)
        {
            Result += A[Index] * B[Index];
        }
        return Result;
    }

    template <typename T>
    static T CrossDot(const TVector3<T>& A, const TVector3<T>& B, const TVector3<T>& P)
    {
        return Math::Dot(Math::Cross(A, B), P);
    }

    /* Distance between two points in 3D space */
    template <typename T>
    static T Distance(const TVector3<T> V0, const TVector3<T>& V1)
    {
        T A = Math::Square(V1.X - V0.X);
        T B = Math::Square(V1.Y - V0.Y);
        T C = Math::Square(V1.Z - V0.Z);
        return Math::Sqrt(A + B + C);
    }

    template <typename T>
    static T Dot(const TVector4<T>& A, const TVector4<T>& B)
    {
        T Result = T(0);
        for (int32 Index = 0; Index < 4; Index++)
        {
            Result += A[Index] * B[Index];
        }
        return Result;
    }

    template <typename T>
    static T Area(const TVector3<T>& V0, const TVector3<T>& V1, const TVector3<T>& V2)
    {
        T A = V0.X * (V1.Y - V2.Y);
        T B = V1.X * (V2.Y - V0.Y);
        T C = V2.X * (V0.Y - V1.Y);
        return Math::Abs((A + B + C) / T(2));
    }

    // Vector Sign
    template <typename T>
    static bool EdgeSign(const TVector2<T>& A, const TVector2<T>& B, const TVector2<T>& C)
    {
        T Result = (C[0] - A[0]) * (B[1] - A[1]) - (C[1] - A[1]) * (B[0] - A[0]);
        return Math::Sign(Result);
    }

    template <typename T>
    static float EdgeValue(const TVector2<T>& A, const TVector2<T>& B, const TVector2<T>& P)
    {
        return Math::Cross(B - A, P - A);
    }

    // Muller-Trumbore ray triangle intersect
    template <typename T>
    static bool GetBarycentric(const TVector3<T>& P,
                               const TVector3<T>& V0, const TVector3<T>& V1, const TVector3<T>& V2,
                               TVector3<T>& UVW,
                               T Tolerance = P_VERY_SMALL_NUMBER)
    {
        const TVector3<T> BA = V1 - V0;
        const TVector3<T> CA = V2 - V0;
        const TVector3<T> PA = P - V0;

        const T D00 = Math::Dot(BA, BA);
        const T D01 = Math::Dot(BA, CA);
        const T D11 = Math::Dot(CA, CA);
        const T D20 = Math::Dot(PA, BA);
        const T D21 = Math::Dot(PA, CA);
        const T Denom = T(1) / (D00 * D11 - D01 * D01);

        const T V = (D11 * D20 - D01 * D21) * Denom;
        const T W = (D00 * D21 - D01 * D20) * Denom;
        const T U = T(1) - V - W;

        UVW.X = U;
        UVW.Y = V;
        UVW.Z = W;

        T Sum = UVW.X + UVW.Y + UVW.Z;
        T OneMinusSum = T(1) - Sum;
        return (
            UVW.X > T(0) &&
            UVW.Y > T(0) &&
            UVW.Z > T(0) &&
            (T(1) - Sum) < Tolerance);
    }

    template <typename T>
    static EWindingOrder GetVertexOrder(const TVector3<T>& V0, const TVector3<T>& V1, const TVector3<T>& V2)
    {
        const float Result = (V1.Y - V0.Y) * (V2.X - V1.X) - (V1.X - V0.X) * (V2.Y - V1.Y);
        if (Result == T(0))
        {
            return EWindingOrder::CL;
        }
        return Result > T(0) ? EWindingOrder::CW : EWindingOrder::CCW;
    }

    template <typename T>
    static void GetLine(const TVector3<T>& A, const TVector3<T>& B, std::vector<IVector2>& Points, int32 Width, int32 Height)
    {
        int32 X0 = static_cast<int32>(A.X);
        int32 Y0 = static_cast<int32>(A.Y);
        int32 X1 = static_cast<int32>(B.X);
        int32 Y1 = static_cast<int32>(B.Y);
        int32 DeltaX = Abs(X1 - X0);
        int32 DeltaY = Abs(Y1 - Y0);
        int32 StepX = (X0 < X1) ? 1 : -1;
        int32 StepY = (Y0 < Y1) ? 1 : -1;
        int32 Error = DeltaX - DeltaY;

        while (true)
        {
            if (X0 >= 0 && X0 < Width && Y0 >= 0 && Y0 < Height)
            {
                Points.emplace_back(X0, Y0);
            }

            if (X0 == X1 && Y0 == Y1)
            {
                break;
            }

            const int32 DoubleError = Error * 2;
            if (DoubleError > -DeltaY)
            {
                Error -= DeltaY;
                X0 += StepX;
            }
            if (DoubleError < DeltaX)
            {
                Error += DeltaX;
                Y0 += StepY;
            }
        }
    }

    template <typename T>
    static T GetDepth(const TVector3<T>& P, const TVector3<T>& V0, const TVector3<T>& V1, const TVector3<T>& V2)
    {
        // Calculate area of this triangle
        T A = Math::Area(V0, V1, V2);

        // Calculate depth
        T W0 = Math::Area(V1, V2, P);
        T W1 = Math::Area(V2, V0, P);
        T W2 = Math::Area(V0, V1, P);

        if (W0 < T(0) && W1 < T(0) && W2 < T(0))
        {
            return FLT_MAX;
        }

        W0 /= A;
        W1 /= A;
        W2 /= A;

        return W0 * V0.Z + W1 * V1.Z + W2 * V2.Z;
    }

    // // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
    template <typename T>
    static TVector3<T> GetSurfaceNormal(const TVector3<T>& V0, const TVector3<T>& V1, const TVector3<T>& V2)
    {
        TVector3<T> Edge0 = V1 - V0;
        TVector3<T> Edge1 = V2 - V0;
        TVector3<T> Normal = Math::Cross(Edge0, Edge1);
        return Normal.Normalized();
    }
};
