#pragma once

#include "MathFwd.h"

struct FColor;
struct FLinearColor;

struct FColor
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
    FColor(uint8 InR, uint8 InG, uint8 InB, uint8 InA = 255)
    {
        R = InR;
        G = InG;
        B = InB;
        A = InA;
    }

public:
    static FColor Red() { return FromRgba(255, 0, 0); }
    static FColor Green() { return FromRgba(0, 255, 0); }
    static FColor Blue() { return FromRgba(0, 0, 255); }

    static FColor Yellow() { return FromRgba(255, 255, 0); }
    static FColor Magenta() { return FromRgba(255, 0, 255); }
    static FColor Cyan() { return FromRgba(0, 255, 255); }

    static FColor White() { return FromRgba(255, 255, 255); }
    static FColor Gray() { return FromRgba(128,128,128); }
    static FColor Black() { return FromRgba(0, 0, 0); }

    static FColor FromRgba(uint8 R, uint8 G, uint8 B, uint8 A = 255) { return {R, G, B, A}; }
};

struct FLinearColor
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

    static FLinearColor FromRgba(float R, float G, float B, float A = 1.0f)
    {
        FLinearColor Color;
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
