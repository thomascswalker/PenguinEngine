#pragma once

#include "MathFwd.h"

struct PColor;
struct PLinearColor;

struct PColor
{
    union
    {
        struct // NOLINT
        {
            uint8 R;
            uint8 G;
            uint8 B;
            uint8 A;
        };
        uint8 RGBA[4];
    };

protected:
    PColor(uint8 InR, uint8 InG, uint8 InB, uint8 InA = 255)
    {
        R = InR;
        G = InG;
        B = InB;
        A = InA;
    }

public:
    static PColor Red() { return FromRgba(255, 0, 0); }
    static PColor Blue() { return FromRgba(0, 255, 0); }
    static PColor Green() { return FromRgba(0, 0, 255); }

    static PColor Yellow() { return FromRgba(255, 255, 0); }
    static PColor Cyan() { return FromRgba(0, 255, 255); }
    static PColor Magenta() { return FromRgba(255, 0, 255); }

    static PColor White() { return FromRgba(255, 255, 255); }
    static PColor Gray() { return FromRgba(128,128,128); }
    static PColor Black() { return FromRgba(0, 0, 0); }

    static PColor FromRgba(uint8 R, uint8 G, uint8 B, uint8 A = 255) { return {R, G, B, A}; }
};

struct PLinearColor
{
    union
    {
        struct // NOLINT
        {
            float R;
            float G;
            float B;
            float A;
        };
        float RGBA[4];
    };

    static PLinearColor FromRgba(float R, float G, float B, float A = 1.0f)
    {
        PLinearColor Color;
        Color.R = R;
        Color.G = G;
        Color.B = B;
        Color.A = A;
        return Color;
    }
    
    void Clamp()
    {
        for (float& V : RGBA)
        {
            V = V > 1.0f ? 1.0f : V;
        }
    }

};
