#include "Math/Color.h"

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
