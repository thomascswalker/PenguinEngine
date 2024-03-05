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

    TPlane(T InX, T InY, T InZ, T InW) : X(InX), Y(InY), Z(InZ), W(InW){}
};