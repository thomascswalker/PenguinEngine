#pragma once

#include "Vector.h"

template <typename T> struct TRect;
typedef TRect<float> Rect;

template <typename T>
struct TRect
{
    TVector2<T> Min;
    TVector2<T> Max;

    TRect(){}
    TRect(TVector2<T> InMin, TVector2<T> InMax) : Min(InMin), Max(InMax){}
};
