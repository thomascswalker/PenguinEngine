#include "Framework/Renderer/Viewport.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Input/InputHandler.h"
#include "Math/MathCommon.h"
#include "Math/Vector.h"

// View Camera
FMatrix PCamera::ComputeViewProjectionMatrix()
{
    const float Scale = 1.0f / Math::Tan(Math::DegreesToRadians(Fov / 2.0f));
    ViewMatrix = FLookAtMatrix(-GetTranslation(), LookAt, FVector3::UpVector());
    ProjectionMatrix = FPerspectiveMatrix(Scale, GetAspect(), MinZ, MaxZ);
    return ProjectionMatrix * ViewMatrix * FMatrix::GetIdentity();
}


void PCamera::Orbit(const float DX, const float DY)
{
    // Convert degrees to radians
    const float Yaw = Math::DegreesToRadians(DX);
    const float Pitch = Math::DegreesToRadians(DY);

    // Get the camera's direction
    FVector3 ViewTranslation = InitialTransform.Translation;
    FVector3 ViewDirection = (ViewTranslation - LookAt).Normalized();

    // Individually rotate the direction by X (Yaw), then Y (Pitch)
    const FMatrix RotX = FMatrix::MakeFromX(0);
    ViewDirection = RotX * ViewDirection;
    
    const FMatrix RotY = FMatrix::MakeFromY(Yaw);
    ViewDirection = RotY * ViewDirection;

    const FMatrix RotZ = FMatrix::MakeFromZ(Pitch);
    ViewDirection = RotZ * ViewDirection;

    // Set the final translation to be the new rotated direction * the zoom distance
    const float ViewDistance = Math::Distance(InitialTransform.Translation, LookAt);
    ViewTranslation = ViewDirection * ViewDistance;
    SetTranslation(ViewTranslation);

    // Construct a new rotator from the LookAt matrix
    FRotator ViewRotation = (RotX * RotY * RotZ).GetRotator();
    ViewRotation.Roll = 0.0f; // Force Roll to always be 0
    SetRotation(ViewRotation);

    // Force scale to always be [1,1,1]
    SetScale(FVector3(1, 1, 1));
}

void PCamera::Pan(float DX, float DY)
{
    const float PanSpeed = 0.2f;
    // Find out which way is forward
    FVector3 ViewTranslation = InitialTransform.Translation;
    const FRotator ViewRotation = InitialTransform.Rotation;
    const FRotator RollRotation(0, 0, ViewRotation.Roll);

    // Get right vector
    const FVector3 RightDirection = FRotationMatrix(RollRotation).GetAxisNormalized(EAxis::Z);
    FVector3 RightOffset = RightDirection * DX * PanSpeed;

    // Get up vector
    const FVector3 UpDirection = FRotationMatrix(RollRotation).GetAxisNormalized(EAxis::Y);
    FVector3 UpOffset = UpDirection * DY * PanSpeed;

    FVector3 Offset = RightOffset + UpOffset;
    SetTranslation(ViewTranslation + Offset);
    SetLookAt(InitialLookAt + Offset);
}

void PCamera::Zoom(float Value)
{
    FVector3 ViewTranslation = InitialTransform.Translation;
    FVector3 ViewDirection = (ViewTranslation - LookAt).Normalized();
    float ViewDistance = Math::Distance(ViewTranslation, LookAt);
    ViewDistance = Math::Max(DEFAULT_MIN_ZOOM, ViewDistance + (Value * 0.25f));
    SetTranslation(ViewDirection * ViewDistance);
}

void PCamera::SetFov(float NewFov)
{
    Fov = Math::Clamp(NewFov, MinFov, MaxFov);
}

// Viewport

PViewport::PViewport(const uint32 InWidth, const uint32 InHeight)
{
    Camera = std::make_shared<PCamera>();
    Resize(InWidth, InHeight);
}

void PViewport::Resize(uint32 InWidth, uint32 InHeight) const
{
    Camera->Width = InWidth;
    Camera->Height = InHeight;
}

FVector2 PViewport::GetSize() const
{
    return {static_cast<float>(Camera->Width), static_cast<float>(Camera->Height)};
}

void PViewport::ResetView()
{
    Camera->SetTranslation(DEFAULT_CAMERA_TRANSLATION);
    Camera->SetRotation(FRotator());
    Camera->InitialLookAt = FVector3::ZeroVector();
    UpdateViewProjectionMatrix();
}

void PViewport::UpdateViewProjectionMatrix()
{
    MVP = Camera->ComputeViewProjectionMatrix();
}

/**
 * \brief Projects the specified `WorldPosition` into the in/out `ScreenPosition` using the specified `ViewProjectionMatrix`.
 * \param WorldPosition The world position of the point to be projected.
 * \param ScreenPosition The out screen position.
 * \return True if the position could be projected, false otherwise.
 */
bool PViewport::ProjectWorldToScreen(const FVector3& WorldPosition, FVector3& ScreenPosition) const
{
    // Clip space
    const FVector4 Result = MVP * FVector4(WorldPosition, 1.0f);
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
            NormalizedPosition.X * static_cast<float>(Camera->Width),
            NormalizedPosition.Y * static_cast<float>(Camera->Height),
            ClipPosition.Z
        };
        return true;
    }

    return false;
}

bool PViewport::ProjectScreenToWorld(const FVector2& ScreenPosition, float Depth, FVector3& WorldPosition)
{
    FMatrix InvMatrix = MVP.GetInverse();
    int32 PixelX = Math::Truncate(ScreenPosition.X);
    int32 PixelY = Math::Truncate(ScreenPosition.Y);
    
    const float NormalizedX = (float)PixelX / (float)GetWidth();
    const float NormalizedY = (float)PixelY / (float)GetHeight();

    const float ScreenSpaceX = (NormalizedX - 0.5f) * 2.0f;
    const float ScreenSpaceY = ((1.0f - NormalizedY) - 0.5f) * 2.0f;

    const FVector4 RayStartProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 1.0f, 1.0f);
    const FVector4 RayEndProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 0.01f, 1.0f);
    
    return true;
}

void PViewport::FormatDebugText()
{
    const PEngine* Engine = PEngine::GetInstance();
    const IInputHandler* InputHandler = IInputHandler::GetInstance();
    std::string MousePosition = InputHandler->GetCurrentCursorPosition().ToString();
    std::string MouseDelta = (InputHandler->GetCurrentCursorPosition() - InputHandler->GetClickPosition()).ToString();

    auto Renderer = Engine->GetRenderer();

    DebugText = std::format(
        "Stats\n"
        "FPS: {}\n"
        "Size: {}\n\n"
        "Controls\n"
        "Wireframe (F1): {}\n"
        "Shaded (F2): {}\n"
        "Depth (F3): {}\n"
        ,
        Engine->GetFps(),
        GetSize().ToString(),
        Renderer->GetRenderFlag(ERenderFlags::Wireframe),
        Renderer->GetRenderFlag(ERenderFlags::Shaded),
        Renderer->GetRenderFlag(ERenderFlags::Depth)
    );
}
