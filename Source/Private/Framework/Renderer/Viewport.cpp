#include "Framework/Renderer/Viewport.h"
#include "Framework/Core/Logging.h"

FMatrix PViewInfo::ComputeViewProjectionMatrix(bool bLookAt)
{
    if (bLookAt)
    {
        return FLookAtMatrix(Translation, FVector3(0,0,0), FVector3::UpVector());
    }

    ViewRotationMatrix = FInverseRotationMatrix(Rotation);
    ViewRotationMatrix = FTranslationMatrix(-Translation);// * ViewRotationMatrix;

    const float HalfFovRad = Math::DegreesToRadians(Fov / 2.0f);
    ProjectionMatrix = FReversedZPerspectiveMatrix(HalfFovRad, Width, Height, MaxZ); // NOLINT
    
    return ViewRotationMatrix * ProjectionMatrix;
}

PViewport::PViewport(const uint32 InWidth, const uint32 InHeight, const FVector3& StartTranslation, const FRotator& StartRotation)
{
    Info = std::make_shared<PViewInfo>();
    Resize(InWidth, InHeight);
    SetViewTranslation(StartTranslation);
    SetViewRotation(StartRotation);
}

void PViewport::Resize(uint32 InWidth, uint32 InHeight) const
{
    LOG_DEBUG("Viewport resized to [{}, {}]", InWidth, InHeight)
    Info->Width = InWidth;
    Info->Height = InHeight;
}

void PViewport::SetViewTranslation(const FVector3& NewTranslation) const
{
    Info->Translation = NewTranslation;
}

void PViewport::AddViewTranslation(const FVector3& Delta) const
{
    Info->Translation += Delta;
}

void PViewport::SetViewRotation(const FRotator& NewRotation) const
{
    Info->Rotation = NewRotation;
}
void PViewport::AddViewRotation(const FRotator& Rotation) const
{
    Info->Rotation += Rotation;
}

void PViewport::UpdateViewProjectionMatrix(bool bLookAt)
{
    MVP = Info->ComputeViewProjectionMatrix(bLookAt);
}


/**
 * \brief Same purpose as the other ProjectWorldToScreen, but uses the current ViewProjectionMatrix of the viewport.
 * This assumes it has already been pre-computed and set.
 * \param WorldPosition The world position of the point to be projected.
 * \param ScreenPosition The out screen position.
 * \return True if the position could be projected, false otherwise.
 */
bool PViewport::ProjectWorldToScreen(const FVector3& WorldPosition, FVector3& ScreenPosition) const
{
    return ProjectWorldToScreen(WorldPosition, MVP, ScreenPosition);
}


/**
 * \brief Projects the specified `WorldPosition` into the in/out `ScreenPosition` using the specified `ViewProjectionMatrix`.
 * \param WorldPosition The world position of the point to be projected.
 * \param ViewProjectionMatrix The view projection matrix of the current view (View * Projection)
 * \param ScreenPosition The out screen position.
 * \return True if the position could be projected, false otherwise.
 */
bool PViewport::ProjectWorldToScreen(const FVector3& WorldPosition, const FMatrix& ViewProjectionMatrix, FVector3& ScreenPosition) const
{
    const FVector4 Result = ViewProjectionMatrix * FVector4(WorldPosition, 1.0f);
    if (Result.W > 0.0f)
    {
        // Normalized device coordinates
        const float NormalizedX = (Result.X / (Result.W * 2.0f)) + 0.5f;
        const float NormalizedY = 1.0f - (Result.Y / (Result.W * 2.0f)) - 0.5f;
        const float NormalizedZ = Result.Z != 0.0f ? 1.0f / Result.Z : 0.0f;

        // Apply the current render width and height
        ScreenPosition = FVector3(NormalizedX * static_cast<float>(Info->Width),
                                  NormalizedY * static_cast<float>(Info->Height),
                                  NormalizedZ);
        return true;
    }

    return false;
}
