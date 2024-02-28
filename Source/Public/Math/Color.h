#pragma once

#include "Types.h"

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

    static PColor FromRgba(const uint8 R, const uint8 G, const uint8 B, const uint8 A = 255);
    void Clamp();
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

    static PLinearColor FromRgba(float R, float G, float B, float A = 1.0f);
    void Clamp();
};
