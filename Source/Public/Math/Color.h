#pragma once

#include "Types.h"

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

    static PColor FromRgba(const uint8 R, const uint8 G, const uint8 B, const uint8 A = 255)
    {
        PColor Color;
        Color.R = R;
        Color.G = G;
        Color.B = B;
        Color.A = A;
        return Color;
    }
};
