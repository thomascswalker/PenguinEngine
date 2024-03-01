#pragma once
#include "Math/Matrix.h"
#include "Math/Types.h"
#include "Math/Vector.h"

#define DEFAULT_VIEWPORT_WIDTH 640
#define DEFAULT_VIEWPORT_HEIGHT 480

// Synonymous with a camera view
struct PViewInfo
{
    uint32 Width = DEFAULT_VIEWPORT_WIDTH;
    uint32 Height = DEFAULT_VIEWPORT_HEIGHT;
    float Fov = 90.0f; // Default FOV

    PVector3 ViewOrigin;
    PMatrix4 ProjectionMatrix;
    PMatrix4 ViewRotationMatrix;

    constexpr float GetAspect() const { return static_cast<float>(Width) / static_cast<float>(Height); }

    PMatrix4 ComputeViewProjectionMatrix()
    {
        ViewRotationMatrix = {
            PPlane(1.0f, 0.0f, 0.0f, -ViewOrigin.X),
            PPlane(0.0f, 1.0f, 0.0f, -ViewOrigin.Y),
            PPlane(0.0f, 0.0f, 1.0f, -ViewOrigin.Z),
            PPlane(0.0f, 0.0f, 0.0f, 1.0f)
        };
        ProjectionMatrix = PMatrix4::ProjectionMatrix(
            Math::Max(0.001f, Fov) * P_PI / 360.0f,
            GetAspect(),
            1.0f,
            1.0f
        );
        return ViewRotationMatrix * ProjectionMatrix;
    }
};

class PViewport
{
    std::shared_ptr<PViewInfo> Info;

public:
    PMatrix4 MVP;
    
    PViewport(uint32 InWidth, uint32 InHeight);
    void Resize(uint32 InWidth, uint32 InHeight) const;

    uint32 GetWidth() const { return Info->Width; }
    uint32 GetHeight() const { return Info->Height; }
    PVector2 GetSize() const { return PVector2(Info->Width, Info->Height); }  // NOLINT

    PViewInfo* GetInfo() const { return Info.get(); }
    bool ProjectWorldToScreen(const PVector3& WorldPosition, const PMatrix4& ViewProjectionMatrix, PVector3& ScreenPosition) const;
};
