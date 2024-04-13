#pragma once

#include "Framework/Engine/Object.h"
#include "Math/MathCommon.h"

#define DEFAULT_VIEWPORT_WIDTH 320
#define DEFAULT_VIEWPORT_HEIGHT 240

#define DEFAULT_FOV 90.0f
#define DEFAULT_MINZ 1.0f
#define DEFAULT_MAXZ 1000.0f

#define DEFAULT_ZOOM 10.0f
#define MIN_ZOOM 5.0f
#define MAX_ZOOM 30.0f

#define DEFAULT_CAMERA_TRANSLATION(X) FVector3(0,0,(X))

enum EViewportType
{
    Perspective,
    Othographic
};

// Synonymous with a camera view
class PCamera : public PObject
{
public:
    uint32 Width = DEFAULT_VIEWPORT_WIDTH;
    uint32 Height = DEFAULT_VIEWPORT_HEIGHT;
    float Fov = DEFAULT_FOV;
    float MinZ = 1.0f;
    float MaxZ = 10.0f;
    float Zoom = DEFAULT_ZOOM;
    float MinZoom = MIN_ZOOM;
    float MaxZoom = MAX_ZOOM;
    float MinFov = 20.0f;
    float MaxFov = 120.0f;

    FVector3 TargetTranslation = FVector3::ZeroVector(); // Origin

    FTransform OriginalTransform;
    
    FMatrix ProjectionMatrix;
    FMatrix ViewMatrix;

    PCamera()
    {
        Init();
    }
    void Init()
    {
        SetTranslation(DEFAULT_CAMERA_TRANSLATION(Zoom));
    }
    constexpr float GetAspect() const { return static_cast<float>(Width) / static_cast<float>(Height); }
    FMatrix ComputeViewProjectionMatrix();
    void Orbit(float DX, float DY);
    void SetFov(float NewFov)
    {
        Fov = Math::Clamp(NewFov, MinFov, MaxFov);
    }
    void SetZoom(float NewZoom)
    {
        Zoom = Math::Clamp(NewZoom, MinZoom, MaxZoom);
    }
};

class PViewport
{
    std::shared_ptr<PCamera> Camera;
    FMatrix MVP;
    bool bShowDebugText = true;
    std::string DebugText;

public:
    FTransform Transform;

    PViewport(uint32 InWidth, uint32 InHeight);
    void Resize(uint32 InWidth, uint32 InHeight) const;

    uint32 GetWidth() const { return Camera->Width; }
    uint32 GetHeight() const { return Camera->Height; }
    FVector2 GetSize() const;
    float GetAspect() const { return static_cast<float>(Camera->Width) / static_cast<float>(Camera->Height); }
    PCamera* GetCamera() const { return Camera.get(); }

    void ResetView();
    void UpdateViewProjectionMatrix();
    FMatrix* GetViewProjectionMatrix() { return &MVP; }
    bool ProjectWorldToScreen(const FVector3& WorldPosition, FVector3& ScreenPosition) const;

    void FormatDebugText();
    std::string GetDebugText() const { return DebugText; }
    bool GetShowDebugText() const { return bShowDebugText; }
    void ToggleShowDebugText() { bShowDebugText = !bShowDebugText; }
};
