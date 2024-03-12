#pragma once

template <typename T>
struct TPlane
{
    union
    {
        struct // NOLINT
        {
            T X;
            T Y;
            T Z;
            T W;
        };
        T XYZW[4];
    };

    TPlane(T V[4]) : X(V[0]), Y(V[1]), Z(V[2]), W(V[3])
    {
    }
    TPlane(T InX, T InY, T InZ, T InW) : X(InX), Y(InY), Z(InZ), W(InW)
    {
    }

    TPlane operator*(T Scalar) const
    {
        TPlane Out(*this);
        Out.X *= Scalar;
        Out.Y *= Scalar;
        Out.Z *= Scalar;
        Out.W *= Scalar;
        return Out;
    }

    TPlane& operator*(T Scalar)
    {
        *this = *this * Scalar;
        return *this;
    }

    T operator[](int32 Index) const { return XYZW[Index]; }
    T& operator[](int32 Index) { return XYZW[Index]; }
};
