#pragma once

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
        TVector2<T> BBMin;
        TVector2<T> BBMax;

        // Min
        BBMin.X = V0.X;
        BBMin.Y = V0.Y;
        BBMax.X = V0.X;
        BBMax.Y = V0.Y;

        // Max (Width)
        if (V1.X < BBMin.X) { BBMin.X = V1.X; }
        if (V2.X < BBMin.X) { BBMin.X = V2.X; }
        if (V1.X > BBMax.X) { BBMax.X = V1.X; }
        if (V2.X > BBMax.X) { BBMax.X = V2.X; }

        // Max (Height)
        if (V1.Y < BBMin.Y) { BBMin.Y = V1.Y; }
        if (V2.Y < BBMin.Y) { BBMin.Y = V2.Y; }
        if (V1.Y > BBMax.Y) { BBMax.Y = V1.Y; }
        if (V2.Y > BBMax.Y) { BBMax.Y = V2.Y; }

        return TRect(BBMin, BBMax);
    }

};
