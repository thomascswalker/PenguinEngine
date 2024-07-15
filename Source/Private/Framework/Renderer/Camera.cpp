#include "Framework/Renderer/Camera.h"
#include "Framework/Engine/Engine.h"

// View Camera
void PCamera::ComputeViewProjectionMatrix()
{
    FVector3 Translation = Spherical.ToCartesian();
    FVector3 Eye = Translation;
    FVector3 Center = Target;
    FVector3 Up = FVector3::UpVector();

    ViewMatrix = FLookAtMatrix(Eye, Center, Up);
    ProjectionMatrix = FPerspectiveMatrix(Math::DegreesToRadians(Fov), GetAspect(), MinZ, MaxZ);
    ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
    InvViewProjectionMatrix = ViewProjectionMatrix.GetInverse();
}

void PCamera::Orbit(float DX, float DY)
{
    SphericalDelta.Theta = Math::DegreesToRadians(-DX); // Horizontal
    SphericalDelta.Phi = Math::DegreesToRadians(DY);    // Vertical
}

void PCamera::Pan(float DX, float DY)
{
    // Compute target distance
    FVector3 Position = GetTranslation();
    FVector3 Offset = Position - Target;

    // The length of the Offset vector gives us the distance from the camera to the target.
    float TargetDistance = Offset.Length();

    // Next, we need to scale this distance by the tangent of half the field of view.
    // This is because the field of view is measured in degrees, but the tangent function expects an angle in radians.
    // We also divide by the height of the viewport to account for the aspect ratio.
    TargetDistance *= Math::Tan((Fov / 2.0f) * P_PI / 180.0f);

    // Pan left/right
    FVector3 XOffset = {ViewMatrix.M[0][0], ViewMatrix.M[1][0], ViewMatrix.M[2][0]}; // X Rotation, column 0
    XOffset.Normalize();
    XOffset *= DX * TargetDistance / static_cast<float>(Height);
    PanOffset = XOffset;

    // Pan up/down
    FVector3 YOffset = {ViewMatrix.M[0][1], ViewMatrix.M[1][1], ViewMatrix.M[2][1]}; // Y Rotation, column 1
    YOffset.Normalize();
    YOffset *= DY * TargetDistance / static_cast<float>(Height);
    PanOffset += YOffset;
}

void PCamera::Zoom(float Value)
{
    FVector3 Translation = GetTranslation();
    Spherical = FSphericalCoords::FromCartesian(Translation.X, Translation.Y, Translation.Z);
    Spherical.Radius = Math::Max(MinZoom, Math::Min(Spherical.Radius - (Value * 0.1f), MaxZoom));
    Translation = Spherical.ToCartesian();
    SetTranslation(Translation);
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
    Spherical = FSphericalCoords::FromCartesian(Offset.X, Offset.Y, Offset.Z);

    // Offset spherical coordinates by the current spherical delta
    Spherical.Theta += SphericalDelta.Theta;
    Spherical.Phi += SphericalDelta.Phi;

    // Restrict Phi to min/max polar angle to prevent locking
    Spherical.Phi = Math::Max(MinPolarAngle, Math::Min(MaxPolarAngle, Spherical.Phi));
    Spherical.MakeSafe();

    // Set camera rotation pitch/yaw
    FRotator NewRotation(
        Math::RadiansToDegrees(Spherical.Theta), // Yaw
        Math::RadiansToDegrees(Spherical.Phi),   // Pitch
        0.0f                                     // Roll
    );
    SetRotation(NewRotation);

    // Convert spherical coordinates back to position
    Offset = Spherical.ToCartesian();

    // Offset the target position based on the computed PanOffset (PCamera::Pan)
    Target += PanOffset;

    // Set the camera position to the target position + offset
    SetTranslation(Target + Offset);
}

void PCamera::DeprojectScreenToWorld(const FVector2& ScreenPoint, FVector3& OutWorldPosition, FVector3& OutWorldDirection) const
{
    int32 PixelX = static_cast<int32>(ScreenPoint.X);
    int32 PixelY = static_cast<int32>(ScreenPoint.Y);

    // Convert to 0..1
    const float NormalizedX = (PixelX - 0.5f) / static_cast<float>(Width);
    const float NormalizedY = (PixelY - 0.5f) / static_cast<float>(Height);

    // Convert to -1..1
    const float ScreenSpaceX = (NormalizedX - 0.5f) * 2.0f;
    const float ScreenSpaceY = ((1.0f - NormalizedY) - 0.5f) * 2.0f;

    // Starting ray, Z=1, near
    FVector4 RayStartProjectionSpace(ScreenSpaceX, ScreenSpaceY, 1.0f, 1.0f);
    // Ending ray Z=0.1, far, any distance in order to calculate the direction
    FVector4 RayEndProjectionSpace(ScreenSpaceX, ScreenSpaceY, 0.01f, 1.0f);

    //
    FVector4 HomoRayStartWorldSpace = InvViewProjectionMatrix * RayStartProjectionSpace;
    FVector4 HomoRayEndWorldSpace = InvViewProjectionMatrix * RayEndProjectionSpace;
    FVector3 RayStartWorldSpace(HomoRayStartWorldSpace.X, HomoRayStartWorldSpace.Y, HomoRayStartWorldSpace.Z);
    FVector3 RayEndWorldSpace(HomoRayEndWorldSpace.X, HomoRayEndWorldSpace.Y, HomoRayEndWorldSpace.Z);

    if (HomoRayStartWorldSpace.W != 0.0f)
    {
        RayStartWorldSpace /= HomoRayStartWorldSpace.W;
    }
    if (HomoRayEndWorldSpace.W != 0.0f)
    {
        RayEndWorldSpace /= HomoRayEndWorldSpace.W;
    }

    FVector3 RayDirWorldSpace = RayEndWorldSpace - RayStartWorldSpace;
    RayDirWorldSpace = RayDirWorldSpace.Normalized();

    OutWorldPosition = FVector3{RayStartWorldSpace.X, RayStartWorldSpace.Y, RayStartWorldSpace.Z};
    OutWorldDirection = FVector3{RayDirWorldSpace.X, RayDirWorldSpace.Y, RayDirWorldSpace.Z};
}
