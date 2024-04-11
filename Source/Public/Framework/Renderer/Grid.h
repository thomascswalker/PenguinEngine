#pragma once

#include "Math/MathCommon.h"

template <typename T>
struct TLine
{
    TVector2<T> A;
    TVector2<T> B;
    TLine(const TVector2<T>& InA, const TVector2<T>& InB) : A(InA), B(InB)
    {
    }
};

template <typename T>
struct TLine3d
{
    TVector3<T> A;
    TVector3<T> B;
    TLine3d(const TVector3<T>& InA, const TVector3<T>& InB) : A(InA), B(InB)
    {
    }
};

struct FGrid
{
    std::vector<FLine3d> Lines;

    FGrid(float Divisions, float CellSize)
    {
        for (float Step = 0; Step <= Divisions; Step += CellSize)
        {
            Lines.emplace_back(FVector3(-Divisions, 0.0f, Step), FVector3(Divisions, 0.0f, Step));
            Lines.emplace_back(FVector3(-Divisions, 0.0f, -Step), FVector3(Divisions, 0.0f, -Step));
            Lines.emplace_back(FVector3(Step, 0.0f, -Divisions), FVector3(Step, 0.0f, Divisions));
            Lines.emplace_back(FVector3(-Step, 0.0f, -Divisions), FVector3(-Step, 0.0f, Divisions));
        }
    }
};
