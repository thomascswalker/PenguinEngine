﻿#pragma once

#include "Framework/Engine/Object.h"
#include "glm.hpp"

#define DEFAULT_VIEWPORT_WIDTH 656 // 640
#define DEFAULT_VIEWPORT_HEIGHT 519 // 480

#define DEFAULT_FOV 90.0f
#define DEFAULT_MINZ 1.0f
#define DEFAULT_MAXZ 1000.0f

#define DEFAULT_MIN_ZOOM 10.0f
#define DEFAULT_CAMERA_TRANSLATION FVector3(DEFAULT_MIN_ZOOM, DEFAULT_MIN_ZOOM / 2.0f, DEFAULT_MIN_ZOOM)


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
    float MinFov = 20.0f;
    float MaxFov = 120.0f;
    
    FVector3 LookAt = FVector3::ZeroVector(); // Origin
    FVector3 InitialLookAt = LookAt;
    FTransform InitialTransform;
    float InitialViewDistance;

    glm::mat4 ProjectionMatrix;
    glm::mat4 ViewMatrix;
    glm::mat4 ViewProjectionMatrix;

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
    void SetLookAt(const FVector3& NewLookAt) { LookAt = NewLookAt; }
};