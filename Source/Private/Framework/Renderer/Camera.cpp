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
    glm::vec3 Center = {LookAt.X, LookAt.Y, LookAt.Z};
    glm::vec3 Up = {0.0f, -1.0f, 0.0f}; // Negative UP value
    ViewMatrix = glm::lookAt(Eye, Center, Up);
    ProjectionMatrix = glm::perspective(Math::DegreesToRadians(Fov), GetAspect(), MinZ, MaxZ);
    ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void PCamera::Orbit(const float DX, const float DY)
{
    const float Yaw = Math::DegreesToRadians(DX);
    const float Pitch = Math::DegreesToRadians(DY);
    float RotationSpeed = 0.1f;

    glm::mat4 Temp(1.0f);
    glm::mat4 OrbitTranslation = glm::translate(Temp, glm::vec3(Yaw, Pitch, 0.0f));
    glm::mat4 OrbitRotation = glm::rotate(OrbitTranslation, RotationSpeed, glm::vec3(DX, DY, 0.0f));

    glm::vec3 Scale;
    glm::quat Orientation;
    glm::vec3 Translation;
    glm::vec3 Skew;
    glm::vec4 Perspective;
    glm::decompose(OrbitRotation, Scale, Orientation, Translation, Skew, Perspective);
    FVector3 Offset(Translation.x, Translation.y, Translation.z);
    SetTranslation(GetTranslation() + Offset);
}

void PCamera::Pan(float DX, float DY)
{
    const float PanSpeed = 0.01f;
    // Find out which way is forward
    FVector3 ViewTranslation = GetTranslation();
    auto Front = (LookAt - ViewTranslation).Normalized();
    auto Right = (Math::Cross(Front, FVector3::UpVector())).Normalized();
    auto Up = (Math::Cross(Front, Right)).Normalized();

    FVector3 Offset = ((Right * DX) + (Up * DY)) * PanSpeed;
    SetTranslation(ViewTranslation - Offset);
    SetLookAt(LookAt - Offset);
}

void PCamera::Zoom(float Value)
{
    FVector3 ViewTranslation = GetTranslation();
    FVector3 ViewDirection = (ViewTranslation - LookAt).Normalized();
    float ViewDistance = Math::Distance(ViewTranslation, LookAt);
    ViewDistance = Math::Max(DEFAULT_MIN_ZOOM, ViewDistance + (Value * 0.25f));
    SetTranslation(ViewDirection * ViewDistance);
}

void PCamera::SetFov(float NewFov)
{
    Fov = Math::Clamp(NewFov, MinFov, MaxFov);
}
