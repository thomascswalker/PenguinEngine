#include "Framework/Renderer/Viewport.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Input/InputHandler.h"
#include "Math/MathCommon.h"
#include "Math/Vector.h"

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

    Camera->Target = FVector3::ZeroVector();
    Camera->ComputeViewProjectionMatrix();
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
    glm::mat4 Model(1.0f);
    glm::mat4 MVP = Camera->ViewProjectionMatrix * Model;
    glm::vec4 ResultGlm = MVP * glm::vec4(WorldPosition.X, WorldPosition.Y, WorldPosition.Z, 1.0f);
    FVector4 Result{ResultGlm.x, ResultGlm.y, ResultGlm.z, ResultGlm.w};
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
            (Result.Z + 1.0f) * 0.5f
        };
        return true;
    }

    return false;
}

bool PViewport::ProjectScreenToWorld(const FVector2& ScreenPosition, float Depth, FVector3& WorldPosition) const
{
    // FMatrix InvMatrix = Camera->ViewProjectionMatrix;
    // int32 PixelX = Math::Truncate(ScreenPosition.X);
    // int32 PixelY = Math::Truncate(ScreenPosition.Y);
    //
    // const float NormalizedX = (float)PixelX / (float)GetWidth();
    // const float NormalizedY = (float)PixelY / (float)GetHeight();
    //
    // const float ScreenSpaceX = (NormalizedX - 0.5f) * 2.0f;
    // const float ScreenSpaceY = ((1.0f - NormalizedY) - 0.5f) * 2.0f;
    //
    // const FVector4 RayStartProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 1.0f, 1.0f);
    // const FVector4 RayEndProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 0.01f, 1.0f);

    return true;
}

void PViewport::FormatDebugText()
{
    const PEngine* Engine = PEngine::GetInstance();
    const IInputHandler* InputHandler = IInputHandler::GetInstance();
    FVector2 MouseDelta = InputHandler->GetDeltaCursorPosition();

    auto Renderer = Engine->GetRenderer();

    DebugText = std::format(
        "Stats\n"
        "FPS: {}\n"
        "Size: {}\n"
        "Camera Direction: {}\n"
        "Controls\n"
        "Wireframe (F1): {}\n"
        "Shaded (F2): {}\n"
        "Depth (F3): {}\n"
        "Use GLM (F4): {}"
        ,
        Engine->GetFps(),
        GetSize().ToString(),
        GetCamera()->GetForwardVector().ToString(),
        Renderer->Settings.GetRenderFlag(ERenderFlag::Wireframe),
        Renderer->Settings.GetRenderFlag(ERenderFlag::Shaded),
        Renderer->Settings.GetRenderFlag(ERenderFlag::Depth),
        Renderer->Settings.GetUseGlm()
    );
}
