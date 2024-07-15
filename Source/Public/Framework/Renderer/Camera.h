#pragma once

#include "Math/MathFwd.h"
#include "Framework/Engine/Object.h"
#include "Math/Spherical.h"
#include <glm/glm.hpp>

constexpr int32 WINDOW_WIDTH_CLIP = 16;
constexpr int32 WINDOW_HEIGHT_CLIP = 59;
constexpr int32 DEFAULT_VIEWPORT_WIDTH = 512 + WINDOW_WIDTH_CLIP;
constexpr int32 DEFAULT_VIEWPORT_HEIGHT = 512 + WINDOW_HEIGHT_CLIP;

constexpr float DEFAULT_FOV = 54.3f;
constexpr float DEFAULT_MINZ = 0.1f;
constexpr float DEFAULT_MAXZ = 100.0f;

constexpr float DEFAULT_MIN_ZOOM = 10.0f;
const FVector3 DEFAULT_CAMERA_TRANSLATION = FVector3(-36, 30, 34);

enum EViewportType
{
    Perspective,
    Othographic
};

struct PViewData
{
    int32 Width = DEFAULT_VIEWPORT_WIDTH;
    int32 Height = DEFAULT_VIEWPORT_HEIGHT;
    float Fov = DEFAULT_FOV;
    float MinZ = 1.0f;
    float MaxZ = 10.0f;
    float MinFov = 20.0f;
    float MaxFov = 120.0f;
    float MinZoom = 2.0f;
    float MaxZoom = 100.0f;

    FVector3 Target = FVector3::ZeroVector(); // Origin
    FSphericalCoords Spherical;
    FSphericalCoords SphericalDelta;
    float MinPolarAngle = 0.0f;
    float MaxPolarAngle = P_PI;
    FVector3 PanOffset;

    FMatrix ProjectionMatrix;
    FMatrix ViewMatrix;
    FMatrix ViewProjectionMatrix;
    FMatrix InvViewProjectionMatrix;

    FVector3 Direction;
    FVector3 Translation;
};

class PCamera : public PObject
{
public:
    int32 Width = DEFAULT_VIEWPORT_WIDTH;
    int32 Height = DEFAULT_VIEWPORT_HEIGHT;
    float Fov = DEFAULT_FOV;
    float MinZ = 1.0f;
    float MaxZ = 10.0f;
    float MinFov = 20.0f;
    float MaxFov = 120.0f;
    float MinZoom = 2.0f;
    float MaxZoom = 100.0f;

    FVector3 Target = FVector3::ZeroVector(); // Origin
    FSphericalCoords Spherical;
    FSphericalCoords SphericalDelta;
    float MinPolarAngle = 0.0f;
    float MaxPolarAngle = P_PI;
    FVector3 PanOffset;

    FMatrix ProjectionMatrix;
    FMatrix ViewMatrix;
    FMatrix ViewProjectionMatrix;
    FMatrix InvViewProjectionMatrix;

    /**
     * Constructor for the PCamera class.
     * Initializes the camera by calling the Init() function.
     */
    PCamera()
    {
        Init();
    }
    void Init()
    {
        SetTranslation(DEFAULT_CAMERA_TRANSLATION);
    }
    constexpr float GetAspect() const { return static_cast<float>(Width) / static_cast<float>(Height); }
    void ComputeViewProjectionMatrix();
    void Orbit(float DX, float DY);
    void Pan(float DX, float DY);
    void Zoom(float Value);
    void SetFov(float NewFov);
    void SetLookAt(const FVector3& NewLookAt) { Target = NewLookAt; }

    void Update(float DeltaTime) override;

    /* General Math */
    // https://github.com/EpicGames/UnrealEngine/blob/c830445187784f1269f43b56f095493a27d5a636/Engine/Source/Runtime/Engine/Private/SceneView.cpp#L1431
    void DeprojectScreenToWorld(const FVector2& ScreenPoint, FVector3& OutWorldPosition, FVector3& OutWorldDirection) const;

    PViewData GetViewData() const
    {
        PViewData Data;

        Data.Width = Width;
        Data.Height = Height;
        Data.Fov = Fov;
        Data.MinZ = MinZ;
        Data.MaxZ = MaxZ;
        Data.Target = Target;
        Data.Spherical = Spherical;
        Data.ProjectionMatrix = ProjectionMatrix;
        Data.ViewMatrix = ViewMatrix;
        Data.ViewProjectionMatrix = ViewProjectionMatrix;
        Data.InvViewProjectionMatrix = InvViewProjectionMatrix;
        Data.Direction = GetForwardVector();
        Data.Translation = GetTranslation();

        return Data;
    }
};

namespace Math
{
    static bool ProjectWorldToScreen(const FVector3& WorldPosition, FVector3& ScreenPosition, const PViewData& ViewData)
    {
        // Clip 
        FVector4 Result = ViewData.ViewProjectionMatrix * FVector4(WorldPosition, 1.0f);
        if (Result.W > 0.0f)
        {
            // Apply perspective correction
            const FVector3 ClipPosition{
                Result.X / Result.W,
                Result.Y / Result.W,
                Result.Z / Result.W
            };

            // Normalized device coordinates
            const FVector2 NormalizedPosition{
                (ClipPosition.X / 2.0f) + 0.5f,
                (ClipPosition.Y / 2.0f) + 0.5f,
            };

            // Apply the current render width and height
            ScreenPosition = FVector3{
                NormalizedPosition.X * static_cast<float>(ViewData.Width),
                NormalizedPosition.Y * static_cast<float>(ViewData.Height),
                (ClipPosition.Z + 0.5f) * 0.5f
            };
            return true;
        }
        return false;
    }

    static bool DeprojectScreenToWorld(const FVector2& S, const PViewData& ViewData)
    {
        return true;
    }
}
