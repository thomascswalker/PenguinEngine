#pragma once

#include "Math/MathCommon.h"

class PObject
{
protected:
    FTransform Transform;

public:
    FVector3 GetTranslation() const { return Transform.Translation; }
    FRotator GetRotation() const { return Transform.Rotation; }
    FVector3 GetScale() const { return Transform.Scale; }

    void SetTranslation(const FVector3& NewTranslation) { Transform.Translation = NewTranslation; }
    void SetRotation(const FRotator& NewRotation) { Transform.Rotation = NewRotation; }
    void SetScale(const FVector3& NewScale) { Transform.Scale = NewScale; }

    FVector3 GetForwardVector() const { return Transform.ToMatrix().GetScaledAxis(EAxis::X); }
    FVector3 GetRightVector() const { return Transform.ToMatrix().GetScaledAxis(EAxis::Z); }
    FVector3 GetUpVector() const { return Transform.ToMatrix().GetScaledAxis(EAxis::Y); }
};