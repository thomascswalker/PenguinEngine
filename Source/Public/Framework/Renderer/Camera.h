#pragma once

#include "Math/MathFwd.h"
#include "Framework/Engine/Object.h"
#include "Math/Spherical.h"

constexpr int32 g_windowWidthClip = 16;
constexpr int32 g_windowHeightClip = 59;
constexpr int32 g_defaultViewportWidth = 640 + g_windowWidthClip;
constexpr int32 g_defaultViewportHeight = 480 + g_windowHeightClip;

constexpr float g_defaultFov = 54.3f;
constexpr float g_defaultMinz = 0.1f;
constexpr float g_defaultMaxz = 100.0f;

constexpr float g_defaultMinZoom = 10.0f;
const auto g_defaultCameraTranslation = FVector3(-36, 30, 34);

enum EViewportType
{
	Perspective,
	Othographic
};

struct PViewData
{
	int32 Width = g_defaultViewportWidth;
	int32 Height = g_defaultViewportHeight;
	float Fov = g_defaultFov;
	float MinZ = 1.0f;
	float MaxZ = 10.0f;
	float MinFov = 20.0f;
	float MaxFov = 120.0f;
	float MinZoom = 2.0f;
	float MaxZoom = 100.0f;

	FVector3 Target = FVector3::ZeroVector(); // Origin
	FSphericalCoords Spherical;
	FSphericalCoords SphericalDelta;
	float MinPolarAngle = 0.0f;
	float MaxPolarAngle = P_PI;
	FVector3 PanOffset;

	FMatrix ProjectionMatrix;
	FMatrix ViewMatrix;
	FMatrix ViewProjectionMatrix;
	FMatrix InvViewProjectionMatrix;

	FVector3 Direction;
	FVector3 Translation;
};

class PCamera : public PObject
{
public:
	int32 m_width = g_defaultViewportWidth;
	int32 m_height = g_defaultViewportHeight;
	float m_fov = g_defaultFov;
	float m_minZ = 1.0f;
	float m_maxZ = 10.0f;
	float m_minFov = 20.0f;
	float m_maxFov = 120.0f;
	float m_minZoom = 2.0f;
	float m_maxZoom = 100.0f;

	FVector3 m_target = FVector3::ZeroVector(); // Origin
	FSphericalCoords m_spherical;
	FSphericalCoords m_sphericalDelta;
	float m_minPolarAngle = 0.0f;
	float m_maxPolarAngle = P_PI;
	FVector3 m_panOffset;

	FMatrix m_projectionMatrix;
	FMatrix m_viewMatrix;
	FMatrix m_viewProjectionMatrix;
	FMatrix m_invViewProjectionMatrix;

	/**
	 * Constructor for the PCamera class.
	 * Initializes the camera by calling the Init() function.
	 */
	PCamera()
	{
		init();
	}

	void init()
	{
		SetTranslation(g_defaultCameraTranslation);
	}

	constexpr float getAspect() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }
	void computeViewProjectionMatrix();
	void orbit(float dx, float dy);
	void pan(float dx, float dy);
	void zoom(float value);
	void setFov(float newFov);
	void setLookAt(const FVector3& newLookAt) { m_target = newLookAt; }

	void Update(float deltaTime) override;

	/* General Math */
	// https://github.com/EpicGames/UnrealEngine/blob/c830445187784f1269f43b56f095493a27d5a636/Engine/Source/Runtime/Engine/Private/SceneView.cpp#L1431
	void deprojectScreenToWorld(const FVector2& screenPoint, FVector3& outWorldPosition,
	                            FVector3& outWorldDirection) const;

	PViewData getViewData() const
	{
		PViewData data;

		data.Width = m_width;
		data.Height = m_height;
		data.Fov = m_fov;
		data.MinZ = m_minZ;
		data.MaxZ = m_maxZ;
		data.Target = m_target;
		data.Spherical = m_spherical;
		data.ProjectionMatrix = m_projectionMatrix;
		data.ViewMatrix = m_viewMatrix;
		data.ViewProjectionMatrix = m_viewProjectionMatrix;
		data.InvViewProjectionMatrix = m_invViewProjectionMatrix;
		data.Direction = GetForwardVector();
		data.Translation = GetTranslation();

		return data;
	}
};

namespace Math
{
	static bool projectWorldToScreen(const FVector3& worldPosition, FVector3& screenPosition, const PViewData& viewData)
	{
		// Clip 
		const FVector4 result = viewData.ViewProjectionMatrix * FVector4(worldPosition, 1.0f);
		if (result.W > 0.0f)
		{
			// Apply perspective correction
			const FVector3 clipPosition{
				result.X / result.W,
				result.Y / result.W,
				result.Z / result.W
			};

			// Normalized device coordinates
			const FVector2 normalizedPosition{
				(clipPosition.X / 2.0f) + 0.5f,
				(clipPosition.Y / 2.0f) + 0.5f,
			};

			// Apply the current render width and height
			screenPosition = FVector3{
				normalizedPosition.X * static_cast<float>(viewData.Width),
				normalizedPosition.Y * static_cast<float>(viewData.Height),
				(clipPosition.Z + 0.5f) * 0.5f
			};
			return true;
		}
		return false;
	}

	static bool deprojectScreenToWorld(const FVector2& screenPosition, const PViewData& viewData)
	{
		return true;
	}
}
