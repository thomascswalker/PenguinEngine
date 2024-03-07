#pragma once

#include "Math/Vector.h"

struct PMouseTracker
{
    bool bIsTracking = false;
    FVector2 Start;
    FVector2 End;
    FVector2 RawDelta;

    bool IsTracking() const { return bIsTracking; }

    void StartTracking(const FVector2& ScreenPosition)
    {
        bIsTracking = true;
        Start = ScreenPosition;
    }

    void EndTracking(const FVector2& ScreenPosition)
    {
        bIsTracking = false;
        End = ScreenPosition;
    }

    void AddDelta(const int32 InDelta)
    {
        RawDelta += InDelta;
    }

    FVector2 GetDelta() const
    {
        const FVector2 Delta(End - Start);
        return Delta;
    }
};
