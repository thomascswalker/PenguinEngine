#include "Framework/Renderer/Viewport.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Input/InputHandler.h"

// Needed for TQuat<float>::Rotator()
#include "../Private/Math/MathCommon.cpp" // NOLINT

// View Camera
FTransform PCamera::GetViewTransform() const
{
    return Transform;
}

FMatrix PCamera::ComputeViewProjectionMatrix()
{
    const float Scale = 1.0f / Math::Tan(Math::DegreesToRadians(Fov / 2.0f));
    ViewMatrix = FLookAtMatrix(GetTranslation(), TargetTranslation, FVector3::UpVector());
    ProjectionMatrix = FPerspectiveMatrix(Scale, GetAspect(), MinZ, MaxZ);
    return ViewMatrix * ProjectionMatrix;
}

void PCamera::Orbit(float DX, float DY, const FVector3& Target)
{
    // Get the camera rotation as a vector
    const FVector3 Direction = (Target - GetTranslation()).Normalized(); // Target is always [0,0,0]

    // Rotate the orientation by the delta X and delta Y values (in degrees)
    const FVector3 RotatedDirection = Math::Rotate(Direction, DX, DY, 0.0f);

    // Multiply by zoom distance to get the final location
    const FVector3 NewTranslation = RotatedDirection * Zoom;

    // Set new location as the current location
    SetTranslation(NewTranslation);
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
        const float NormalizedZ = Result.Z != 0.0f ? 1.0f / Result.Z : 0.0f;

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
    std::string MouseDelta = InputHandler->GetDeltaCursorPosition().ToString();

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

    FVector3 Translation = Camera->GetViewTransform().Translation;
    FRotator Rotation = Camera->GetViewTransform().Rotation;

    DebugText = std::format(
        "Click and drag to orbit\n"
        "Press 'T' to toggle debug text\n\n"
        "FPS: {}\n"
        "Size: {}\n"
        "Mouse Position: {}\n"
        "Mouse Delta: {}\n"
        "Keys pressed: {}\n\n"
        "Camera Position: {}\n"
        "Camera zoom: {}"
        ,
        Engine->GetFps(),
        GetSize().ToString(),
        MousePosition,
        MouseDelta,
        FmtKeysDown,
        Translation.ToString(),
        Camera->Zoom
    );
}
