#include "Framework/Renderer/Camera.h"
#include "Framework/Engine/Engine.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm.hpp"
#include "gtx/matrix_decompose.hpp"
#include "gtc/matrix_transform.hpp"

// View Camera
void PCamera::ComputeViewProjectionMatrix()
{
    FVector3 Translation = GetTranslation();
    glm::vec3 Eye = {Translation.X, Translation.Y, Translation.Z};
    glm::vec3 Center = {Target.X, Target.Y, Target.Z};
    glm::vec3 Up = {0.0f, -1.0f, 0.0f}; // Negative UP value
    ViewMatrix = glm::lookAt(Eye, Center, Up);
    ProjectionMatrix = glm::perspective(Math::DegreesToRadians(Fov), GetAspect(), MinZ, MaxZ);
    ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void PCamera::Orbit(const float DX, const float DY)
{
    SphericalDelta.Theta = Math::DegreesToRadians(DX); // Horizontal
    SphericalDelta.Phi = Math::DegreesToRadians(DY); // Vertical
}

void PCamera::Pan(float DX, float DY)
{
    float DampingFactor = 0.1f;

    // Compute target distance
    FVector3 Position = GetTranslation();
    FVector3 Offset = Position - Target;
    float TargetDistance = Offset.Length();

    // Scale target distance to account for FOV
    TargetDistance *= Math::Tan((Fov / 2.0f) * P_PI / 180.0f);

    // Pan left/right
    FVector3 XOffset = {ViewMatrix[0][0], ViewMatrix[0][1], ViewMatrix[0][2]}; // X Rotation, column 0
    XOffset.Normalize();
    XOffset *= DX * DampingFactor * TargetDistance / static_cast<float>(Height);
    PanOffset = XOffset;

    // Pan up/down
    FVector3 YOffset = {ViewMatrix[1][0], ViewMatrix[1][1], ViewMatrix[1][2]}; // Y Rotation, column 1
    YOffset.Normalize();
    YOffset *= DY * DampingFactor * TargetDistance / static_cast<float>(Height);
    PanOffset += YOffset;
}

void PCamera::Zoom(float Value)
{
    Spherical.Radius -= Value;
}

void PCamera::SetFov(float NewFov)
{
    Fov = Math::Clamp(NewFov, MinFov, MaxFov);
}

void PCamera::Update(float DeltaTime)
{
    // Get the offset from the current camera position to the target position
    FVector3 Position = GetTranslation();
    FVector3 Offset = Position - Target;

    // Convert offset to spherical coordinates
    Spherical.SetFromCartesian(Offset.X, Offset.Y, Offset.Z);

    // Offset spherical coordinates by the current spherical delta
    Spherical.Theta += SphericalDelta.Theta;
    Spherical.Phi += SphericalDelta.Phi;

    // Restrict Phi to min/max polar angle to prevent locking
    Spherical.Phi = Math::Max(MinPolarAngle, Math::Min(MaxPolarAngle, Spherical.Phi));
    Spherical.MakeSafe();

    // Convert spherical coordinates back to position
    Offset = Spherical.ToCartesian();

    // Offset the target position based on the computed PanOffset (PCamera::Pan)
    Target += PanOffset;

    // Set the camera position to the target position + offset
    SetTranslation(Target + Offset);
}
