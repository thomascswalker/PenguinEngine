#pragma once

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
    virtual ~PObject() = default;
    virtual void Update(float DeltaTime){};
    
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
    FVector3 GetForwardVector() const
    {
        FRotator R = Transform.Rotation;
        FVector3 Forward;
        Forward.X = Math::Sin(R.Yaw);
        Forward.Y = -(Math::Sin(R.Pitch) * Math::Cos(R.Yaw));
        Forward.Z = -(Math::Cos(R.Pitch) * Math::Cos(R.Yaw));
        return Forward;
    }
    FVector3 GetRightVector() const
    {
        const FVector3 Forward = GetForwardVector();
        return (Math::Cross(Forward, FVector3::UpVector())).Normalized();
    }
    FVector3 GetUpVector() const
    {
        const FVector3 Forward = GetForwardVector();
        const FVector3 Right = GetRightVector();
        return (Math::Cross(Forward, Right)).Normalized();
    }
};
