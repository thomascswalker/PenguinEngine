#pragma once

#include "Math/MathCommon.h"

struct PLine3
{
    FVector3 A;
    FVector3 B;
    PLine3(const FVector3& InA, const FVector3& InB) : A(InA), B(InB)
    {
    }
};

struct PGrid
{
    std::vector<PLine3> Lines;

    PGrid(float Count, float Size)
    {
        for (float Step = 0; Step <= Count; Step += Size)
        {
            Lines.emplace_back(FVector3(-Count, 0.0f, Step), FVector3(Count, 0.0f, Step));
            Lines.emplace_back(FVector3(-Count, 0.0f, -Step), FVector3(Count, 0.0f, -Step));
            Lines.emplace_back(FVector3(Step, 0.0f, -Count), FVector3(Step, 0.0f, Count));
            Lines.emplace_back(FVector3(-Step, 0.0f, -Count), FVector3(-Step, 0.0f, Count));
        }
    }
};
