#pragma once

template <typename T> struct TRect;
typedef TRect<int> RectI;
typedef TRect<float> RectF;
typedef TRect<double> RectD;

template <typename T>
struct TRect
{
    T Min[2];
    T Max[2];

    TRect(){}
    TRect(T InMin[2], T InMax[2]) : Min(InMin), Max(InMax){}
};
