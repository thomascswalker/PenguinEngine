#pragma once

#include "Camera.h"
#include "Math/MathCommon.h"

class PViewport
{
    std::shared_ptr<PCamera> Camera;
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
    bool ProjectWorldToScreen(const FVector3& WorldPosition, FVector3& ScreenPosition) const;
    bool ProjectScreenToWorld(const FVector2& ScreenPosition, float Depth, FVector3& WorldPosition) const;

    void FormatDebugText();
    std::string GetDebugText() const { return DebugText; }
    bool GetShowDebugText() const { return bShowDebugText; }
    void ToggleShowDebugText() { bShowDebugText = !bShowDebugText; }
};
