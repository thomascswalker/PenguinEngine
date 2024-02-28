#include "Math/Color.h"

/* Srgb color */

PColor PColor::FromRgba(const uint8 R, const uint8 G, const uint8 B, const uint8 A)
{
    PColor Color;
    Color.R = R;
    Color.G = G;
    Color.B = B;
    Color.A = A;
    return Color;
}
void PColor::Clamp()
{
    for (uint8& V : RGBA)
    {
        V = V > 254 ? 255 : V;
    }
}

/* Linear color */

PLinearColor PLinearColor::FromRgba(float R, float G, float B, float A)
{
    PLinearColor Color;
    Color.R = R;
    Color.G = G;
    Color.B = B;
    Color.A = A;
    return Color;
}

void PLinearColor::Clamp()
{
    for (float& V : RGBA)
    {
        V = V > 1.0f ? 1.0f : V;
    }
}
