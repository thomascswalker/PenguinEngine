#pragma once

#include "Framework/Engine/Object.h"
#include "Math/Spherical.h"
#include "glm.hpp"

constexpr int32 WINDOW_WIDTH_CLIP = 16;
constexpr int32 WINDOW_HEIGHT_CLIP = 39;
constexpr int32 DEFAULT_VIEWPORT_WIDTH = 512 + WINDOW_WIDTH_CLIP;
constexpr int32 DEFAULT_VIEWPORT_HEIGHT = 512 + WINDOW_HEIGHT_CLIP;

constexpr float DEFAULT_FOV = 54.3f;
constexpr float DEFAULT_MINZ = 0.1f;
constexpr float DEFAULT_MAXZ = 100.0f;

constexpr float DEFAULT_MIN_ZOOM = 10.0f;
const FVector3 DEFAULT_CAMERA_TRANSLATION = FVector3(36, 30, 34);


enum EViewportType
{
    Perspective,
    Othographic
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

    glm::mat4 ProjectionMatrix;
    glm::mat4 ViewMatrix;
    glm::mat4 ViewProjectionMatrix;

    FVector3 TempVector;

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
    FVector3 GetDirectionVector() const { return (Target - GetTranslation()).Normalized(); }

    void Update(float DeltaTime) override;
};
