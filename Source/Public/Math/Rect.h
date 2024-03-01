#pragma once

#include "MathFwd.h"

template <typename T>
struct TRect
{
    struct
    {
        T X;
        T Y;
        T Width;
        T Height;
    };

    TRect(){}
    TRect(T InX, T InY, T InWidth, T InHeight)
    {
        X = InX;
        Y = InY;
        Width = InX + InWidth;
        Height = InY + InHeight;
    }
};
