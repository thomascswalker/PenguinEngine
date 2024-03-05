#pragma once

#include "Math/MathCommon.h"

#define DEFAULT_VIEWPORT_WIDTH 320
#define DEFAULT_VIEWPORT_HEIGHT 240

enum EViewportType
{
    Perspective,
    Othographic
};

// Synonymous with a camera view
struct PViewInfo
{
    uint32 Width = DEFAULT_VIEWPORT_WIDTH;
    uint32 Height = DEFAULT_VIEWPORT_HEIGHT;
    float Fov = 90.0f; // Default FOV
    float MinZ = 1.0f;
    float MaxZ = 1000.0f;

    FVector3 Translation;
    FRotator Rotation;
    FVector3 LookAt;

    FMatrix ProjectionMatrix;
    FMatrix ViewRotationMatrix;

    constexpr float GetAspect() const { return static_cast<float>(Width) / static_cast<float>(Height); }

    FTransform GetViewTransform()
    {
        return {Rotation, Translation};
    }
    FMatrix ComputeViewProjectionMatrix(bool bLookAt = false);
};

class PViewport
{
    std::shared_ptr<PViewInfo> Info;
    FMatrix MVP;

public:
    FTransform Transform;

    PViewport(uint32 InWidth, uint32 InHeight, const FVector3& StartTranslation = FVector3::ZeroVector(), const FRotator& StartRotation = FRotator::Identity());
    void Resize(uint32 InWidth, uint32 InHeight) const;

    uint32 GetWidth() const { return Info->Width; }
    uint32 GetHeight() const { return Info->Height; }
    FVector2 GetSize() const
    {
        return {static_cast<float>(Info->Width), static_cast<float>(Info->Height)};
    }

    PViewInfo* GetInfo() const { return Info.get(); }
    
    void SetViewTranslation(const FVector3& NewTranslation) const;
    void AddViewTranslation(const FVector3& Delta) const;
    void SetViewRotation(const FRotator& NewRotation) const;
    void AddViewRotation(const FRotator& Rotation) const;

    void UpdateViewProjectionMatrix(bool bLookAt = false);
    FMatrix* GetViewProjectionMatrix() { return &MVP; }
    bool ProjectWorldToScreen(const FVector3& WorldPosition, FVector3& ScreenPosition) const;
    bool ProjectWorldToScreen(const FVector3& WorldPosition, const FMatrix& ViewProjectionMatrix, FVector3& ScreenPosition) const;
};
