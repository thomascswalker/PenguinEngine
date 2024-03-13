﻿#pragma once

#include "Math/MathCommon.h"

enum class ECoordinateSpace
{
    Local,
    World
};

class PObject
{
protected:
    FTransform Transform;

public:
    // Getters
    FTransform GetTransform() const { return Transform; }
    FVector3 GetTranslation() const { return Transform.Translation; }
    FRotator GetRotation() const { return Transform.Rotation; }
    FVector3 GetScale() const { return Transform.Scale; }

    // Setters
    void SetTranslation(const FVector3& NewTranslation) { Transform.Translation = NewTranslation; }
    void SetRotation(const FRotator& NewRotation) { Transform.Rotation = NewRotation; }
    void SetScale(const FVector3& NewScale) { Transform.Scale = NewScale; }

    // Manipulators
    void Translate(const FVector3& Delta, ECoordinateSpace Space = ECoordinateSpace::World)
    {
        Transform.Translation += Delta;
    }
    void Rotate(float Pitch, float Yaw, float Roll)
    {
        Transform.Rotation += FRotator(Pitch, Yaw, Roll);
        Transform.Rotation.Normalize();
    }

    // Axes
    FVector3 GetForwardVector() const { return Transform.ToMatrix().GetAxis(EAxis::X); }
    FVector3 GetRightVector() const { return Transform.ToMatrix().GetAxis(EAxis::Z); }
    FVector3 GetUpVector() const { return Transform.ToMatrix().GetAxis(EAxis::Y); }
};