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
	Camera->m_width = InWidth;
	Camera->m_height = InHeight;
}

FVector2 PViewport::GetSize() const
{
	return {static_cast<float>(Camera->m_width), static_cast<float>(Camera->m_height)};
}

void PViewport::ResetView()
{
	Camera->SetTranslation(g_defaultCameraTranslation);
	Camera->SetRotation(FRotator());

	Camera->m_target = FVector3::ZeroVector();
	Camera->computeViewProjectionMatrix();
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
	FMatrix Model;
	FMatrix MVP = Camera->m_viewProjectionMatrix * Model;
	FVector4 Result = MVP * FVector4(WorldPosition.X, WorldPosition.Y, WorldPosition.Z, 1.0f);
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
			NormalizedPosition.X * static_cast<float>(Camera->m_width),
			NormalizedPosition.Y * static_cast<float>(Camera->m_height),
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

void PViewport::formatDebugText()
{
	const PEngine* engine = PEngine::getInstance();
	const IInputHandler* inputHandler = IInputHandler::GetInstance();

	DebugText = std::format(
		"Stats\n"
		"FPS: {}\n"
		"Size: {}\n",
		engine->getFps(),
		GetSize().ToString()
	);
}
