#include "Framework/Renderer/Viewport.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Input/InputHandler.h"

// View Camera
FMatrix PCamera::ComputeViewProjectionMatrix()
{
    const float Scale = 1.0f / Math::Tan(Math::DegreesToRadians(Fov / 2.0f));
    ViewMatrix = FLookAtMatrix(-GetTranslation(), TargetTranslation, FVector3::UpVector());
    ProjectionMatrix = FPerspectiveMatrix(Scale, GetAspect(), MinZ, MaxZ);
    return ProjectionMatrix * ViewMatrix * FMatrix::GetIdentity();
}

void PCamera::Orbit(const float DX, const float DY)
{
    // Convert degrees to radians
    const float RX = Math::DegreesToRadians(DX);
    const float RY = Math::DegreesToRadians(DY);

    // Get the camera's direction
    FVector3 Direction = (OriginalTransform.Translation - TargetTranslation).Normalized();

    // Individually rotate the direction by X (Yaw), then Y (Pitch)
    const FMatrix RotY = FMatrix::MakeFromY(RX);
    Direction = RotY * Direction;

    const FMatrix RotZ = FMatrix::MakeFromZ(RY);
    Direction = RotZ * Direction;

    // Set the final translation to be the new rotated direction * the zoom distance
    const FVector3 NewTranslation = Direction * Zoom;
    SetTranslation(NewTranslation);

    // Construct a new rotator from the LookAt matrix
    FRotator NewRotator = (RotY * RotZ).GetRotator();
    NewRotator.Roll = 0.0f; // Force Roll to always be 0
    SetRotation(NewRotator);

    // Force scale to always be [1,1,1]
    SetScale(FVector3(1, 1, 1));
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
    Camera->SetTranslation(DEFAULT_CAMERA_TRANSLATION(Camera->Zoom));
    Camera->SetRotation(FRotator());
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
