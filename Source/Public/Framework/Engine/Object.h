#pragma once

#include "Math/MathCommon.h"
#include "Math/Spherical.h"

enum class ECoordinateSpace
{
    Local,
    World
};

class PObject
{
protected:
    FTransform Transform;

    // Basis vectors
    FVector3 ForwardVector;
    FVector3 RightVector;
    FVector3 UpVector;

public:
    virtual ~PObject() = default;
    virtual void Update(float DeltaTime)
    {
    }

    void ComputeBasisVectors()
    {
        FSphericalCoords Temp = FSphericalCoords::FromRotation(Transform.Rotation);
        ForwardVector = Temp.ToCartesian().Normalized();
        RightVector = Math::Cross(FVector3::UpVector(), ForwardVector).Normalized();
        UpVector = Math::Cross(ForwardVector, RightVector).Normalized();
    }

    // Getters
    FTransform GetTransform() const { return Transform; }
    FVector3 GetTranslation() const { return Transform.Translation; }
    FRotator GetRotation() const { return Transform.Rotation; }
    FVector3 GetScale() const { return Transform.Scale; }

    // Setters
    void SetTranslation(const FVector3& NewTranslation) { Transform.Translation = NewTranslation; }
    void SetRotation(const FRotator& NewRotation)
    {
        Transform.Rotation = NewRotation;
        ComputeBasisVectors();
    }
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
        ComputeBasisVectors();
    }

    // Axes

    FVector3 GetForwardVector() const
    {
        return ForwardVector;
    }
    FVector3 GetRightVector() const
    {
        return RightVector;
    }
    FVector3 GetUpVector() const
    {
        return UpVector;
    }
};
