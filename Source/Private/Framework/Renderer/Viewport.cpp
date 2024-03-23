#include "Framework/Renderer/Viewport.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Input/InputHandler.h"

// View Camera
FMatrix PCamera::ComputeViewProjectionMatrix()
{
    const float Scale = 1.0f / Math::Tan(Math::DegreesToRadians(Fov / 2.0f));
    ViewMatrix = FLookAtMatrix(GetTranslation(), TargetTranslation, FVector3::UpVector());
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
        const float NormalizedZ = Result.Z != 0.0f ? 1.0f / (Result.Z / (Result.W * 2.0f)) : 0.0f;

        // Apply the current render width and height
        ScreenPosition = FVector3(NormalizedX * static_cast<float>(Camera->Width),
                                  NormalizedY * static_cast<float>(Camera->Height),
                                  NormalizedZ);
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

    auto KeysDown = InputHandler->GetKeysDown();
    std::string FmtKeysDown;

    for (size_t Index = 0; Index < KeysDown.size(); ++Index)
    {
        FmtKeysDown += KeysDown[Index];
        if (Index != KeysDown.size() - 1)
        {
            FmtKeysDown += ", ";
        }
    }

    FVector3 Translation = Camera->GetTranslation();
    FRotator Rotation = Camera->GetRotation();

    DebugText = std::format(
        "Click and drag to orbit\n"
        "Press 'T' to toggle debug text\n\n"
        "FPS: {}\n"
        "Size: {}\n"
        "Mouse Position: {}\n"
        "Mouse Delta: {}\n"
        "Keys pressed: {}\n\n"
        "Camera Position: {}\n"
        "Camera Rotation: {}\n"
        "Camera zoom: {}"
        ,
        Engine->GetFps(),
        GetSize().ToString(),
        MousePosition,
        MouseDelta,
        FmtKeysDown,
        Translation.ToString(),
        Rotation.ToString(),
        Camera->Zoom
    );
}
