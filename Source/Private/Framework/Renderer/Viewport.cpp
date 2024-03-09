#include "Framework/Renderer/Viewport.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Input/InputHandler.h"

// Needed for TQuat<float>::Rotator()
#include "../Private/Math/MathCommon.cpp" // NOLINT

// View Camera
FTransform PCamera::GetViewTransform()
{
    return {Rotation, Translation};
}

FMatrix PCamera::ComputeViewProjectionMatrix(bool bLookAt)
{
    if (bLookAt)
    {
        ViewMatrix = FLookAtMatrix(Translation, TargetTranslation, UpVector);
    }
    else
    {
        ViewMatrix = FMatrix::MakeRotationMatrix(Rotation);
        ViewMatrix = FTranslationMatrix(Translation) * ViewMatrix;
    }

    const float Scale = 1.0f / Math::Tan(Math::DegreesToRadians(Fov / 2.0f));
    ProjectionMatrix = FPerspectiveMatrix(Scale, GetAspect(), MinZ, MaxZ); // NOLINT

    return ViewMatrix * ProjectionMatrix;
}

void PCamera::Orbit(float DX, float DY)
{
    // Get the camera rotation as a vector
    const FVector3 CameraOrientation = (Translation - TargetTranslation).GetSafeNormal(); // Target is always [0,0,0]

    // Convert delta X and Y to radians
    const float Pitch = Math::DegreesToRadians(DX); // DX * (P_PI / 180.f)
    const float Yaw = Math::DegreesToRadians(DY); // DY * (P_PI / 180.f)

    // Construct a quaternion with the derived Pitch and Yaw
    FQuat QuatRotation{Pitch, Yaw, 0};
            
    // Construct a quaternion with the derived camera orientation
    const FQuat QuatTranslation(CameraOrientation.X, CameraOrientation.Y, CameraOrientation.Z, 0);
            
    // Multiply Qr * Qt * Qr⁻¹
    const FQuat QuatResult = QuatRotation * QuatTranslation * QuatRotation.GetConjugate();
            
    // Take X/Y/Z components of the result and apply them as the X/Y/Z components of
    // the new rotated orientation
    const FVector3 RotatedOrientation{QuatResult.X, QuatResult.Y, QuatResult.Z};

    // Multiply by zoom distance to get the final location
    const FVector3 NewTranslation = RotatedOrientation * Zoom;

    // Set new location as the current location
    Translation = NewTranslation;
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

void PViewport::SetViewTranslation(const FVector3& NewTranslation) const
{
    Camera->Translation = NewTranslation;
}

void PViewport::AddViewTranslation(const FVector3& Delta) const
{
    Camera->Translation += Delta;
}

void PViewport::SetViewRotation(const FRotator& NewRotation) const
{
    Camera->Rotation = NewRotation;
}
void PViewport::AddViewRotation(const FRotator& Rotation) const
{
    Camera->Rotation += Rotation;
}

void PViewport::UpdateViewProjectionMatrix(bool bLookAt)
{
    MVP = Camera->ComputeViewProjectionMatrix(bLookAt);
}


/**
 * \brief Same purpose as the other ProjectWorldToScreen, but uses the current ViewProjectionMatrix of the viewport.
 * This assumes it has already been pre-computed and set.
 * \param WorldPosition The world position of the point to be projected.
 * \param ScreenPosition The out screen position.
 * \return True if the position could be projected, false otherwise.
 */
bool PViewport::ProjectWorldToScreen(const FVector3& WorldPosition, FVector3& ScreenPosition)
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
bool PViewport::ProjectWorldToScreen(const FVector3& WorldPosition, const FMatrix& ViewProjectionMatrix, FVector3& ScreenPosition)
{
    FMatrix Model; // Default model matrix
    Model.SetIdentity();
    
    const FVector4 Result = Camera->ViewMatrix * Camera->ProjectionMatrix * FVector4(WorldPosition, 1.0f);
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
        "Camera Rotation: {}"
        ,
        Engine->GetFps(),
        GetSize().ToString(),
        MousePosition,
        MouseDelta,
        FmtKeysDown,
        Translation.ToString(),
        Rotation.ToString(),
        MVP.ToString()
    );
}
