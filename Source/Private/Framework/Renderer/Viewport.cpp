#include "Framework/Renderer/Viewport.h"

#include "Framework/Core/Logging.h"

PViewport::PViewport(uint32 InWidth, uint32 InHeight)
{
    Info = std::make_shared<PViewInfo>();
    Resize(InWidth, InHeight);
}

void PViewport::Resize(uint32 InWidth, uint32 InHeight) const
{
    LOG_DEBUG("Viewport resized to [{}, {}]", InWidth, InHeight)
    Info->Width = InWidth;
    Info->Height = InHeight;
}

bool PViewport::ProjectWorldToScreen(const PVector3& WorldPosition, const PMatrix4& ViewProjectionMatrix, PVector3& ScreenPosition) const
{
    const PVector4 Result = ViewProjectionMatrix * PVector4(WorldPosition, 1.0f);
    if (Result.W > 0.0f)
    {
        const float NormalizedX = (Result.X / (Result.W * 2.0f)) + 0.5f;
        const float NormalizedY = 1.0f - (Result.Y / (Result.W * 2.0f)) - 0.5f;
        const float NormalizedZ = Result.Z != 0.0f ? 1.0f / Result.Z : 0;
        ScreenPosition = PVector3(NormalizedX * static_cast<float>(Info->Width),
                                  NormalizedY * static_cast<float>(Info->Height),
                                  NormalizedZ);
        return true;
    }

    return false;
}
