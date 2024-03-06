﻿#pragma once

#include "Vector.h"

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

    TRect()
    {
    }
    TRect(const TVector2<T>& InMin, const TVector2<T>& InMax)
    {
        X = InMin.X;
        Y = InMin.Y;
        Width = InMax.X - InMin.X;
        Height = InMax.Y - InMin.Y;
    }
    TRect(T InX, T InY, T InWidth, T InHeight)
    {
        X = InX;
        Y = InY;
        Width = InWidth;
        Height = InHeight;
    }

    TVector2<T> Min() const { return TVector2(X, Y); }
    TVector2<T> Max() const { return TVector2(X + Width, Y + Height); }

    static TRect MakeBoundingBox(const TVector2<T>& V0, const TVector2<T>& V1, const TVector2<T>& V2)
    {
        T XValues[3] = {V0.X, V1.X, V2.X};
        T YValues[3] = {V0.Y, V1.Y, V2.Y};
        
        const T MinX = *std::ranges::min_element(XValues);
        const T MinY = *std::ranges::min_element(YValues);
        const T MaxX = *std::ranges::max_element(XValues);
        const T MaxY = *std::ranges::max_element(YValues);

        TVector2<T> BBMin(MinX, MinY);
        TVector2<T> BBMax(MaxX, MaxY);

        return TRect(BBMin, BBMax);
    }
};
