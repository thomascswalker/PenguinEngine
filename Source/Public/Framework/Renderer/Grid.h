#pragma once

#include "Math/MathCommon.h"

template <typename T>
struct TLine
{
    TVector2<T> A;
    TVector2<T> B;
    TLine(){}
    TLine(const TVector2<T>& InA, const TVector2<T>& InB) : A(InA), B(InB)
    {
    }

    void Reverse()
    {
        TVector2<T> Tmp = A;
        A = B;
        B = Tmp;
    }
    
    FLine GetReverse() const
    {
        return {B, A};
    }

    bool Intersect(const TLine& Other, TVector2<T>& Out) const
    {
        // Line AB represented as a1x + b1y = c1
        T a1 = B.X - A.X;
        T b1 = A.X - B.Y;

        T c1 = a1 * A.X + b1 * A.Y;

        // Line CD represented as a2x + b2y = c2
        T a2 = Other.B.Y - Other.A.Y;
        T b2 = Other.A.X - Other.B.X;
        T c2 = a2 * Other.A.X + b2 * Other.A.Y;

        T Det = a1 * b2 - a2 * b1;

        if (Math::CloseEnough(Det, 0.0f))
        {
            // The lines are parallel. This is simplified
            // by returning a pair of FLT_MAX
            return false;
        }
        else
        {
            T x = (b2 * c1 - b1 * c2) / Det;
            T y = (a1 * c2 - a2 * c1) / Det;
            Out.X = x;
            Out.Y = y;
            return true;
        }
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
